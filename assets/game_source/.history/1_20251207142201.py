from PIL import Image
import numpy as np

# 打开图片
img = Image.open('D:/game_sourceplayer.jpg').convert('RGBA')
arr = np.array(img)

# 背景色 (灰色)
bg_color = np.array([197, 204, 212])

# 计算每个像素与背景色的差异
diff = np.sqrt(np.sum((arr[:,:,:3].astype(float) - bg_color.astype(float))**2, axis=2))

# 容差值，差异小于这个值的像素变透明
tolerance = 30

# 创建透明度蒙版
alpha = np.where(diff < tolerance, 0, 255).astype(np.uint8)
arr[:,:,3] = alpha

# 保存为PNG
result = Image.fromarray(arr)
result.save('/mnt/user-data/outputs/player_transparent.png')
print("已生成透明背景的 player_transparent.png")

# 显示处理结果
transparent_pixels = np.sum(alpha == 0)
total_pixels = alpha.size
print(f"透明像素: {transparent_pixels} / {total_pixels} ({transparent_pixels*100/total_pixels:.1f}%)")