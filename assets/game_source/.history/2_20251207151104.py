import os
from PIL import Image

def split_manual_sections(image_path, cut_points):
    """
    模式 A: 根据指定的 Y 轴坐标，将长图横向切成几个大部分
    """
    img = Image.open(image_path)
    width, height = img.size
    filename = os.path.splitext(os.path.basename(image_path))[0]
    
    # 创建输出文件夹
    output_dir = f"{filename}_sections"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # 添加起始点0和结束点height，确保覆盖全图
    points = [0] + sorted(cut_points) + [height]
    
    print(f"--- 开始按区域切割: {image_path} ---")
    
    for i in range(len(points) - 1):
        top = points[i]
        bottom = points[i+1]
        
        # 裁剪区域 (Left, Top, Right, Bottom)
        box = (0, top, width, bottom)
        crop = img.crop(box)
        
        save_name = f"{output_dir}/part_{i+1}.png"
        crop.save(save_name)
        print(f"已保存: {save_name} (高度范围: {top} - {bottom})")
    
    print("--- 区域切割完成 ---")

def split_into_grid(image_path, tile_size=32):
    """
    模式 B: 将图片切成一个个独立的小方块 (如 32x32)
    """
    img = Image.open(image_path)
    width, height = img.size
    filename = os.path.splitext(os.path.basename(image_path))[0]
    
    output_dir = f"{filename}_tiles_{tile_size}px"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    print(f"--- 开始网格切割: {tile_size}x{tile_size} ---")
    
    count = 0
    # 遍历行和列
    for y in range(0, height, tile_size):
        for x in range(0, width, tile_size):
            # 防止超出边界
            if x + tile_size > width or y + tile_size > height:
                continue
                
            box = (x, y, x + tile_size, y + tile_size)
            crop = img.crop(box)
            
            # 保存文件，文件名包含坐标信息
            save_name = f"{output_dir}/tile_{x}_{y}.png"
            crop.save(save_name)
            count += 1
            
    print(f"--- 网格切割完成，共生成 {count} 个图块 ---")

# ================= 配置区域 =================

# 图片文件名 (请确保图片和脚本在同一文件夹)
image_file = "image_f0e218.jpg" 

# 【重要】在这里填入你想切割的 Y 轴像素位置
# 你可以用画图工具打开图片，把鼠标放在你想切分的分界线上，记下 Y 坐标填在这里
# 例如：假设森林在 500像素结束，房屋在 1200像素结束...
manual_cut_y_coords = [512, 1024, 1600, 2200] 

# 这里选择你想用的模式 (注释掉不需要的那行)
if __name__ == "__main__":
    # 模式 A: 切成几个大块 (推荐用于分类)
    split_manual_sections(image_file, manual_cut_y_coords)
    
    # 模式 B: 切成几千个小方块 (推荐用于制作素材库)
    # split_into_grid(image_file, tile_size=32)