from PIL import Image
import numpy as np

# ========== 配置 ==========
input_file = 'D:/game_source/unnamed.jpg'
output_file = 'D:/game_source/unnamed_transparent.png'
tolerance = 30  # 容差值，可以调大（如50）来去除更多边缘

# ========== 打开图片 ==========
img = Image.open(input_file).convert('RGBA')
arr = np.array(img)
print(f"图片尺寸: {img.size[0]} x {img.size[1]}")

# ========== 自动检测背景色 ==========
# 方法：取四个角落的像素，找最常见的颜色作为背景色
corners = [
    arr[0, 0, :3],           # 左上
    arr[0, -1, :3],          # 右上
    arr[-1, 0, :3],          # 左下
    arr[-1, -1, :3],         # 右下
    arr[5, 5, :3],           # 左上偏移
    arr[5, -5, :3],          # 右上偏移
]

# 打印检测到的角落颜色
print("\n检测到的角落颜色:")
for i, c in enumerate(corners):
    print(f"  位置{i+1}: RGB({c[0]}, {c[1]}, {c[2]})")

# 取平均值作为背景色
bg_color = np.mean(corners, axis=0).astype(int)
print(f"\n推测的背景色: RGB({bg_color[0]}, {bg_color[1]}, {bg_color[2]})")

# ========== 创建透明蒙版 ==========
diff = np.sqrt(np.sum((arr[:,:,:3].astype(float) - bg_color.astype(float))**2, axis=2))
alpha = np.where(diff < tolerance, 0, 255).astype(np.uint8)
arr[:,:,3] = alpha

# ========== 保存结果 ==========
result = Image.fromarray(arr)
result.save(output_file)

# ========== 显示统计 ==========
transparent_pixels = np.sum(alpha == 0)
total_pixels = alpha.size
print(f"\n透明像素: {transparent_pixels} / {total_pixels} ({transparent_pixels*100/total_pixels:.1f}%)")
print(f"\n已保存到: {output_file}")

# ========== 如果透明像素太少，给出提示 ==========
if transparent_pixels * 100 / total_pixels < 10:
    print("\n⚠️  警告：透明像素比例很低！")
    print("可能原因：")
    print("  1. 背景色检测不准确 - 手动指定 bg_color")
    print("  2. tolerance 值太小 - 尝试增大到 50 或更高")
    print("\n手动指定背景色示例：")
    print("  bg_color = np.array([255, 255, 255])  # 白色背景")
    print("  bg_color = np.array([0, 0, 0])        # 黑色背景")