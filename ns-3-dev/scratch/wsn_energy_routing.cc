#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/energy-module.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <fstream>
#include <vector>
#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WSN-MultiRouting");

std::vector<bool> nodeDead;
uint32_t sinkPackets = 0;
double firstDeathTime = -1.0;

void RxCallback(Ptr<const Packet>, const Address &) {
    sinkPackets++;
}

void CheckEnergy(Ptr<energy::BasicEnergySource> source, uint32_t nodeId, std::string tag) {
    double energy = source->GetRemainingEnergy();
    std::ofstream f("energy_" + tag + ".csv", std::ios::out | std::ios::app);
    f << Simulator::Now().GetSeconds() << "," << nodeId << "," << energy << "\n";
    f.close();

    if (energy < 0.1 && !nodeDead[nodeId]) {
        nodeDead[nodeId] = true;
        if (firstDeathTime < 0.0) {
            firstDeathTime = Simulator::Now().GetSeconds();
        }
    }

    uint32_t deadCount = std::count(nodeDead.begin(), nodeDead.end(), true);
    std::ofstream deadLog("dead_nodes_" + tag + ".csv", std::ios::out | std::ios::app);
    deadLog << Simulator::Now().GetSeconds() << "," << deadCount << "\n";
    deadLog.close();

    Simulator::Schedule(Seconds(5.0), &CheckEnergy, source, nodeId, tag);
}

void RunSimulation(std::string protocol, std::string dataRateStr) {
    uint32_t nNodes = 50;
    double simTime = 300.0;
    uint32_t packetSize = 64;
    uint32_t numSenders = 10;
    sinkPackets = 0;
    firstDeathTime = -1.0;
    nodeDead.clear();
    nodeDead.resize(nNodes, false);

    std::string tag = protocol + "_" + dataRateStr;

    NodeContainer nodes;
    nodes.Create(nNodes);

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(25),
                                  "DeltaY", DoubleValue(25),
                                  "GridWidth", UintegerValue(5),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);

    YansWifiPhyHelper phy;
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    InternetStackHelper stack;
    Ipv4ListRoutingHelper list;

    if (protocol == "AODV") {
        AodvHelper aodv;
        list.Add(aodv, 100);
    } else if (protocol == "DSDV") {
        DsdvHelper dsdv;
        list.Add(dsdv, 100);
    } else if (protocol == "OLSR") {
        OlsrHelper olsr;
        list.Add(olsr, 100);
    }

    stack.SetRoutingHelper(list);
    stack.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    BasicEnergySourceHelper energy;
    energy.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(100.0));
    energy::EnergySourceContainer sources = energy.Install(nodes);

    WifiRadioEnergyModelHelper radioEnergy;
    radioEnergy.Install(devices, sources);

    uint16_t port = 9;
    OnOffHelper onoff("ns3::UdpSocketFactory",
                      InetSocketAddress(interfaces.GetAddress(0), port));
    onoff.SetConstantRate(DataRate(dataRateStr));
    onoff.SetAttribute("PacketSize", UintegerValue(packetSize));
    onoff.SetAttribute("StartTime", TimeValue(Seconds(1.0)));
    onoff.SetAttribute("StopTime", TimeValue(Seconds(simTime)));

    for (uint32_t i = 1; i <= numSenders; i++) {
        onoff.Install(nodes.Get(i));
    }

    PacketSinkHelper sink("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApp = sink.Install(nodes.Get(0));
    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(simTime));

    Ptr<PacketSink> pktSink = DynamicCast<PacketSink>(sinkApp.Get(0));
    pktSink->TraceConnectWithoutContext("Rx", MakeCallback(&RxCallback));

    for (uint32_t i = 0; i < nNodes; i++) {
        Ptr<energy::BasicEnergySource> src = DynamicCast<energy::BasicEnergySource>(sources.Get(i));
        Simulator::Schedule(Seconds(5.0), &CheckEnergy, src, i, tag);
    }

    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();

    Simulator::Stop(Seconds(simTime + 1.0));
    Simulator::Run();

    monitor->CheckForLostPackets();
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    double totalDelay = 0;
    uint32_t totalRx = 0;
    uint32_t totalTx = 0;

    for (auto &flow : stats) {
        FlowMonitor::FlowStats fs = flow.second;
        totalRx += fs.rxPackets;
        totalTx += fs.txPackets;
        totalDelay += fs.delaySum.GetSeconds();
    }

    double avgDelay = (totalRx > 0) ? totalDelay / totalRx : 0;
    double PDR = (totalTx > 0) ? (double)sinkPackets / totalTx * 100.0 : 0;
    double throughput = (sinkPackets * packetSize * 8) / simTime / 1000.0;

    double totalConsumed = 0;
    uint32_t deadCount = 0;

    for (uint32_t i = 0; i < nNodes; i++) {
        Ptr<energy::BasicEnergySource> src = DynamicCast<energy::BasicEnergySource>(sources.Get(i));
        double remaining = src->GetRemainingEnergy();
        totalConsumed += 100.0 - remaining;
        if (nodeDead[i]) deadCount++;
    }

    std::ofstream result("results_" + tag + ".csv", std::ios::out);
    result << std::fixed << std::setprecision(4)
           << protocol << "," << PDR << "," << avgDelay << "," << throughput << ","
           << totalConsumed << "," << deadCount << "," << firstDeathTime << "\n";
    result.close();

    Simulator::Destroy();
}

int main() {
    std::vector<std::string> protocols = {"AODV", "DSDV", "OLSR"};
    // std::vector<std::string> protocols = {"OLSR"};
    std::vector<std::string> dataRates = {
        "4kbps", "12kbps", "20kbps"
    };

    for (const std::string &protocol : protocols) {
        for (const std::string &rate : dataRates) {
            std::cout << "\n=== Running " << protocol << " with DataRate " << rate << " ===\n";
            RunSimulation(protocol, rate);
        }
    }

    return 0;
}