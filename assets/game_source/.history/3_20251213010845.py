import tkinter as tk
from tkinter import filedialog, messagebox, Listbox, Scrollbar
from PIL import Image, ImageTk
import os

class TileStitcherApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Tiled 图块缝合机 (Visual Stitcher)")
        self.root.geometry("800x600")

        # 数据存储
        self.current_folder = ""
        self.image_files = [] # 文件名列表
        self.slots_data = {
            'TL': None, # 左上 (Image对象, 文件路径)
            'TR': None, # 右上
            'BL': None, # 左下
            'BR': None  # 右下
        }
        self.base_size = None # 记录第一张图的尺寸，用于校验

        # --- UI 布局 ---
        
        # 1. 顶部工具栏
        top_frame = tk.Frame(root, pady=10)
        top_frame.pack(fill=tk.X)
        
        tk.Button(top_frame, text="1. 打开素材文件夹", command=self.load_folder, bg="#dddddd", height=2).pack(side=tk.LEFT, padx=10)
        tk.Button(top_frame, text="重置画布", command=self.reset_slots, height=2).pack(side=tk.LEFT, padx=10)
        tk.Button(top_frame, text="保存最终大图", command=self.save_image, bg="#88ff88", height=2).pack(side=tk.RIGHT, padx=10)

        # 2. 主体区域 (左右分栏)
        main_frame = tk.Frame(root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # 左侧：文件列表
        left_panel = tk.Frame(main_frame, width=250)
        left_panel.pack(side=tk.LEFT, fill=tk.Y)
        
        tk.Label(left_panel, text="点击选择图片:").pack(anchor=tk.W)
        
        self.listbox = Listbox(left_panel, width=30)
        self.listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        scrollbar = Scrollbar(left_panel, command=self.listbox.yview)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.listbox.config(yscrollcommand=scrollbar.set)

        # 右侧：拼接区域
        right_panel = tk.Frame(main_frame, bg="#eeeeee")
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        tk.Label(right_panel, text="点击下方格子填入选中的图片:", bg="#eeeeee").pack(pady=5)

        # 田字格容器
        grid_frame = tk.Frame(right_panel)
        grid_frame.pack(expand=True)

        # 创建4个按钮作为格子
        self.btn_tl = self.create_slot_btn(grid_frame, "左上 (TL)", 'TL', 0, 0)
        self.btn_tr = self.create_slot_btn(grid_frame, "右上 (TR)", 'TR', 0, 1)
        self.btn_bl = self.create_slot_btn(grid_frame, "左下 (BL)", 'BL', 1, 0)
        self.btn_br = self.create_slot_btn(grid_frame, "右下 (BR)", 'BR', 1, 1)

    def create_slot_btn(self, parent, text, position_key, r, c):
        # 创建一个大按钮代表格子
        btn = tk.Button(parent, text=text, width=20, height=10, 
                        command=lambda: self.fill_slot(position_key, btn))
        btn.grid(row=r, column=c, padx=5, pady=5)
        return btn

    def load_folder(self):
        folder = filedialog.askdirectory()
        if not folder:
            return
        
        self.current_folder = folder
        self.image_files = []
        self.listbox.delete(0, tk.END)
        
        valid_exts = ('.png', '.jpg', '.jpeg', '.bmp')
        for f in os.listdir(folder):
            if f.lower().endswith(valid_exts):
                self.image_files.append(f)
                self.listbox.insert(tk.END, f)

    def fill_slot(self, pos_key, btn_widget):
        # 获取当前列表选中的文件
        selection = self.listbox.curselection()
        if not selection:
            messagebox.showwarning("提示", "请先在左侧列表中点击选中一张图片！")
            return
        
        filename = self.listbox.get(selection[0])
        full_path = os.path.join(self.current_folder, filename)
        
        try:
            img = Image.open(full_path)
            
            # 尺寸校验
            if self.base_size is None:
                self.base_size = img.size
            else:
                if img.size != self.base_size:
                    messagebox.showerror("错误", f"图片尺寸不匹配！\n这就尴尬了：\n第一张图是 {self.base_size}\n这张图是 {img.size}")
                    return

            # 保存原始数据用于合并
            self.slots_data[pos_key] = img
            
            # 生成缩略图显示在按钮上
            display_img = img.copy()
            display_img.thumbnail((128, 128)) # 限制缩略图大小
            photo = ImageTk.PhotoImage(display_img)
            
            btn_widget.config(image=photo, text="", width=128, height=128)
            btn_widget.image = photo # 保持引用防止回收
            
        except Exception as e:
            messagebox.showerror("错误", str(e))

    def reset_slots(self):
        self.slots_data = {'TL': None, 'TR': None, 'BL': None, 'BR': None}
        self.base_size = None
        # 重置按钮外观
        for btn, text in [(self.btn_tl, "左上"), (self.btn_tr, "右上"), 
                          (self.btn_bl, "左下"), (self.btn_br, "右下")]:
            btn.config(image='', text=text, width=20, height=10)

    def save_image(self):
        # 检查是否至少有一张图
        if all(v is None for v in self.slots_data.values()):
            messagebox.showwarning("空", "画布是空的，没东西可存。")
            return

        if self.base_size is None:
            return

        w, h = self.base_size
        # 创建大图 (透明背景)
        new_img = Image.new('RGBA', (w * 2, h * 2))

        # 填坑
        if self.slots_data['TL']: new_img.paste(self.slots_data['TL'], (0, 0))
        if self.slots_data['TR']: new_img.paste(self.slots_data['TR'], (w, 0))
        if self.slots_data['BL']: new_img.paste(self.slots_data['BL'], (0, h))
        if self.slots_data['BR']: new_img.paste(self.slots_data['BR'], (w, h))

        save_path = filedialog.asksaveasfilename(defaultextension=".png", filetypes=[("PNG", "*.png")])
        if save_path:
            new_img.save(save_path)
            messagebox.showinfo("成功", "拼接完成！\n现在你可以把这张图拖进 Tiled 了。")

if __name__ == "__main__":
    root = tk.Tk()
    app = TileStitcherApp(root)
    root.mainloop()