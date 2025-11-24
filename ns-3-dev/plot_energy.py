import pandas as pd
import matplotlib.pyplot as plt
import os

# Cấu hình
protocols = ["AODV", "DSDV", "OLSR"]
data_rates = ["4kbps", "12kbps", "20kbps"]
colors = {"AODV": "blue", "DSDV": "orange", "OLSR": "green"}

# === 1. Delay trung bình theo DataRate ===
plt.figure(figsize=(10, 6))
for proto in protocols:
    delays = []
    for rate in data_rates:
        filename = f"results_{proto}_{rate}.csv"
        if os.path.exists(filename):
            df = pd.read_csv(filename, header=None)
            df.columns = ["Protocol", "PDR", "Delay", "Throughput", "Energy", "DeadNodes", "FirstDeath"]
            delays.append(df["Delay"].iloc[0])
        else:
            delays.append(None)
    plt.plot(data_rates, delays, label=proto, color=colors[proto], marker='o')
plt.title("Average Delay vs DataRate")
plt.xlabel("DataRate")
plt.ylabel("Delay (s)")
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig("delay_vs_datarate.png")
plt.close()

# === 2. PDR trung bình theo DataRate ===
plt.figure(figsize=(10, 6))
for proto in protocols:
    pdrs = []
    for rate in data_rates:
        filename = f"results_{proto}_{rate}.csv"
        if os.path.exists(filename):
            df = pd.read_csv(filename, header=None)
            df.columns = ["Protocol", "PDR", "Delay", "Throughput", "Energy", "DeadNodes", "FirstDeath"]
            pdrs.append(df["PDR"].iloc[0])
        else:
            pdrs.append(None)
    plt.plot(data_rates, pdrs, label=proto, color=colors[proto], marker='o')
plt.title("Average PDR vs DataRate")
plt.xlabel("DataRate")
plt.ylabel("Packet Delivery Ratio (%)")
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig("pdr_vs_datarate.png")
plt.close()

# === 3. Throughput trung bình theo DataRate ===
plt.figure(figsize=(10, 6))
for proto in protocols:
    throughputs = []
    for rate in data_rates:
        filename = f"results_{proto}_{rate}.csv"
        if os.path.exists(filename):
            df = pd.read_csv(filename, header=None)
            df.columns = ["Protocol", "PDR", "Delay", "Throughput", "Energy", "DeadNodes", "FirstDeath"]
            throughputs.append(df["Throughput"].iloc[0])
        else:
            throughputs.append(None)
    plt.plot(data_rates, throughputs, label=proto, color=colors[proto], marker='o')
plt.title("Average Throughput vs DataRate")
plt.xlabel("DataRate")
plt.ylabel("Throughput (kbps)")
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig("throughput_vs_datarate.png")
plt.close()

# === 4. Energy Consumed vs DataRate ===
plt.figure(figsize=(10, 6))
for proto in protocols:
    energies = []
    for rate in data_rates:
        filename = f"results_{proto}_{rate}.csv"
        if os.path.exists(filename):
            df = pd.read_csv(filename, header=None)
            df.columns = ["Protocol", "PDR", "Delay", "Throughput", "Energy", "DeadNodes", "FirstDeath"]
            energies.append(df["Energy"].iloc[0])
        else:
            energies.append(None)
    plt.plot(data_rates, energies, label=proto, color=colors[proto], marker='o')
plt.title("Total Energy Consumed vs DataRate")
plt.xlabel("DataRate")
plt.ylabel("Energy Consumed (J)")
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig("energy_vs_datarate.png")
plt.close()

print("✅ Đã vẽ xong 4 biểu đồ: Delay, PDR, Throughput và Energy theo DataRate (không có nhãn giá trị).")