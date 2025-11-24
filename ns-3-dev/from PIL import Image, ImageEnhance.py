from PIL import Image, ImageEnhance
import os

# Thư mục chứa ảnh gốc
input_dir = r"D:\data"
# Thư mục lưu ảnh sau xử lý
output_dir = r"D:\data_output"

# Tạo thư mục output nếu chưa có
os.makedirs(output_dir, exist_ok=True)

for i in range(1, 41):
    filename = f"{i}.jpg"
    input_path = os.path.join(input_dir, filename)
    
    if not os.path.exists(input_path):
        print(f"Không tìm thấy file: {filename}")
        continue

    img = Image.open(input_path)

    # ---- 1. Tăng độ sáng ----
    enhancer = ImageEnhance.Brightness(img)
    brighter = enhancer.enhance(1.3)   # tăng 30%
    brighter.save(os.path.join(output_dir, f"{i}_bright.jpg"))

    # ---- 2. Giảm độ sáng ----
    darker = enhancer.enhance(0.7)     # giảm 30%
    darker.save(os.path.join(output_dir, f"{i}_dark.jpg"))

    # ---- 3. Xoay trái 15 độ ----
    left_rotated = img.rotate(15, expand=True)
    left_rotated.save(os.path.join(output_dir, f"{i}_left15.jpg"))

    # ---- 4. Xoay phải 15 độ ----
    right_rotated = img.rotate(-15, expand=True)
    right_rotated.save(os.path.join(output_dir, f"{i}_right15.jpg"))

    print(f"Đã xử lý xong ảnh: {filename}")

print("Hoàn thành xử lý tất cả ảnh!")
