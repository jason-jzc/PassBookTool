"""
    -------------------------------------------------------------------------
    Author: JiangZongchen
    Email: hello2022_jzc@yeah.net
    Time: 2025.3
    FileName: main.py
    -------------------------------------------------------------------------

    该文件为项目主程序，也是所有功能的入口文件。

"""

# 导入模块
import os
import threading
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext

# 导入配置文件以及API
import config
import API

# 导入字符库
import CharacterLibs as char_lib

# 初始化API
basic_function = API.Basic_Function()

# 定义主要变量
passbook_file_path = None
loaded_data = None


def clear_widgets(root:tk.Tk | tk.Toplevel):
    """
    该函数用于清空窗口内的所有组件
    """
    for widget in root.winfo_children(): # 遍历所有组件并删除
        widget.destroy()

def check_loaded_data():

    """
    该函数用于检查当前密码簿是否加载，该函数将会以多线程运行
    """

    while not check_loaded_data_thread_stop_event.is_set():
        if loaded_data == {} or loaded_data == None or passbook_file_path == "" or passbook_file_path == None:
            status_label.config(text=char_lib.INTERFACE.NOT_LOAD_PASSBOOK_LABLE.value, fg="gray")
        else:
            status_label.config(text=char_lib.INTERFACE.HAS_LOAD_PASSBOOK_LABLE.value, fg="green")
            path_lable.config(text=passbook_file_path, fg="gray")

        check_loaded_data_thread_stop_event.wait(0.3)

def on_closing():
    """
    该函数将会在窗口关闭时执行
    该函数用于退出其他线程并对文件进行保存
    """
    close = messagebox.askyesno(char_lib.TITLE.TIP_WINDOW.value,char_lib.INTERFACE.EXIT_TIP.value)
    if close:
        if loaded_data == {} or loaded_data == None or passbook_file_path == "" or passbook_file_path == None:
            pass
        else:
            MainCommands.save_passbook()
        check_loaded_data_thread_stop_event.set()
        check_loaded_data_thread.join(timeout=2)
        root.destroy()
        if check_loaded_data_thread.is_alive():
            os._exit(0)


class check_email_commands:

    """
    该类为邮箱验证窗口类的扩展类，用于实现更加底层的数据操作
    """

    @staticmethod
    def check_email(email:str, window:tk.Toplevel):

        """
        该函数用于验证输入的邮箱是否与密码簿中的邮箱匹配
        """

        try:
            if basic_function.check_email(InputEmail=email, InputFilePath=passbook_file_path):
                clear_widgets(window)
                check_email_windows.check_vercode_window(window=window)
            else:
                messagebox.showerror(char_lib.TITLE.EMAILECHECK_WINDOW.value, char_lib.INTERFACE.EMAIL_ERROR.value)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

    def check_vercode(vercode:str, window:tk.Toplevel):

        """
        该函数用于验证发送至邮箱的验证码是否正确
        """

        try:
            global loaded_data
            temp_data = basic_function.get_data(InputFilePath=passbook_file_path, VerifyCode=vercode)
            if not temp_data == None:
                loaded_data = temp_data
                window.destroy()
            else:
                messagebox.showerror(char_lib.TITLE.EMAILECHECK_WINDOW.value, char_lib.INTERFACE.VERCODE_ERROR.value)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

class new_passbook_commands:

    """
    该类为新建密码簿窗口类的扩展类，用于实现更加底层的数据操作
    """

    @staticmethod
    def add_email_command(Email:str, window:tk.Toplevel):

        """
        该函数用于新建密码簿后将邮箱添加至密码簿数据中
        """

        try:
            global loaded_data
            chose = messagebox.askokcancel(char_lib.TITLE.WARN_WINDOW.value, char_lib.INTERFACE.EMAIL_CHECK_TIP.value)
            if chose:
                loaded_data = basic_function.new_passbook_data(Email=Email)
                window.destroy()
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

class add_passbook_commands:

    """
    该类为添加密码窗口类的扩展类，用于实现更加底层的数据操作
    """

    @staticmethod
    def add_pass_command(passName:str,account:str,password:str,window:tk.Toplevel):

        """
        该函数用于添加密码至密码簿数据中，同时会进行判断，若名称重复则询问用户是否覆盖
        """

        try:
            global loaded_data

            tempData = basic_function.add_data(Data=loaded_data, Password=password, Account=account, Name=passName)
            if tempData == None:
                chose = messagebox.askyesno(char_lib.TITLE.WARN_WINDOW.value, char_lib.INTERFACE.PASS_HAS_TIP.value)
                if chose:
                    tempData = basic_function.change_data(Data=loaded_data, Password=password, Account=account, Name=passName)
                    if not tempData == None:
                        loaded_data = tempData
                        messagebox.showinfo(char_lib.TITLE.TIP_WINDOW.value, char_lib.INTERFACE.PASS_CHANGE_OK_TIP.value)
                        window.destroy()
                    else:
                        messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, char_lib.INTERFACE.PASS_CHANGE_ERROR_TIP.value)
            else:
                loaded_data = tempData
                messagebox.showinfo(char_lib.TITLE.TIP_WINDOW.value, "密码簿数据添加成功！")
                window.destroy()
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

class search_passbook_commands:

    """
    该类为搜索密码窗口类的扩展类，用于实现更加底层的数据操作
    """

    @staticmethod
    def search_pass_command(passName:str, window:tk.Toplevel, text_area:scrolledtext.ScrolledText):

        """
        该函数用于搜索密码簿数据，并将搜索数据添加至窗口显示
        """

        try:
            search_data = basic_function.search_data(Data=loaded_data, Search=passName)
            if not search_data == None:
                text_area.config(state=tk.NORMAL)
                text_area.delete("1.0", tk.END)

                for i in search_data.keys():
                    text_area.insert(tk.END, f"{i}: \n")
                    text_area.insert(tk.END, f"\t{char_lib.INTERFACE.ACCOUNT_NAME.value}: {search_data[i][config.ACCOUNT_JSON_NAME]}\n")
                    text_area.insert(tk.END, f"\t{char_lib.INTERFACE.PASSWORD_NAME.value}: {search_data[i][config.PASSWORD_JSON_NAME]}\n\n")

                text_area.config(state=tk.DISABLED)
            else:
                messagebox.showinfo(char_lib.TITLE.TIP_WINDOW.value, "未找到相关数据！")
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

class check_email_windows:

    """
    该类为检查邮箱的窗口类，用于实现和用户间的操作
    """

    @staticmethod
    def check_email_window(window:tk.Toplevel):

        """
        该函数用于创建邮箱检查的窗口
        用于提供窗口使得用户可以输入邮箱，并进行验证
        """

        try:
            msg_lable_text = tk.Label(
                window,
                text=char_lib.INTERFACE.INPUT_EMAIL_LABLE.value,
                font=("Arial", 15)
            )
            msg_lable_text.pack(padx=10, pady=10)

            input_box = tk.Entry(
                window,
                width=50,
                font=("Arial", 15)
            )

            input_box.pack(padx=10, pady=10)

            ok_button = tk.Button(
                window,
                text=char_lib.INTERFACE.OK_BUTTON.value,
                font=("Arial", 15),
                command=lambda: check_email_commands.check_email(email=input_box.get(), window=window)
            )

            ok_button.pack(padx=10, pady=10)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

    @staticmethod
    def check_vercode_window(window:tk.Toplevel):

        """
        该函数用于创建窗口，用于输入验证码
        """

        try:
            msg_lable_text = tk.Label(
                window,
                text=char_lib.INTERFACE.VERCODE_HAS_SEND_LABLE.value,
                font=("Arial", 15)
            )
            msg_lable_text.pack(padx=10, pady=10)

            input_box = tk.Entry(
                window,
                width=50,
                font=("Arial", 15)
            )

            input_box.pack(padx=10, pady=10)

            ok_button = tk.Button(
                window,
                text=char_lib.INTERFACE.OK_BUTTON.value,
                font=("Arial", 15),
                command=lambda: check_email_commands.check_vercode(vercode=input_box.get(), window=window)
            )

            ok_button.pack(padx=10, pady=10)

        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))


class new_passbook_windows:

    """
    该类主要是创建新密码簿的时候的次级窗口
    """

    
    @staticmethod
    def add_email_window(window:tk.Toplevel):

        """
        该函数用于创建邮箱添加窗口，主要是当创建新的密码簿时，添加邮箱用
        """

        try:
            msg_lable = tk.Label(
                window,
                text=char_lib.INTERFACE.INPUT_EMAIL_LABLE.value,
                font=("Arial", 15)
            )
            msg_lable.pack(padx=10, pady=10)

            warning_lable = tk.Label(
                window,
                text=char_lib.INTERFACE.NEW_PASSBOOK_EMAIL_WARNING_LABLE.value,
                font=("Arial", 10),
                fg="red"
            )
            warning_lable.pack(padx=10, pady=10)

            input_box = tk.Entry(
                window,
                width=50,
                font=("Arial", 15)
            )
            input_box.pack(padx=10, pady=10)

            ok_button = tk.Button(
                window,
                text=char_lib.INTERFACE.OK_BUTTON.value,
                font=("Arial", 15),
                command=lambda: new_passbook_commands.add_email_command(Email=input_box.get(), window=window)
            )
            ok_button.pack(padx=10, pady=10)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

class add_passbook_windows:

    """
    该类主要是添加/修改密码簿的次级窗口
    """

    @staticmethod
    def add_pass_window(window:tk.Toplevel):

        """
        该函数用于创建窗口，用于添加密码
        """

        try:
            if loaded_data == None or loaded_data == {} or passbook_file_path == None or passbook_file_path == "":
                window.destroy()
                messagebox.showerror(char_lib.TITLE.ERROR_WINDOW.value, char_lib.INTERFACE.NOT_LOAD_PASSBOOK_TIP.value)
            else:
                passName_frame = tk.Frame(window,width=100)

                passName_msg = tk.Label(
                    passName_frame,
                    text=char_lib.INTERFACE.INPUT_PASSNAME_LABLE.value,
                    font=("Arial", 15)
                )
                passName_msg.pack(side=tk.LEFT, padx=10, pady=10)

                passName_input_box = tk.Entry(
                    passName_frame,
                    width=10,
                    font=("Arial", 15)
                )
                passName_input_box.pack(side=tk.RIGHT, padx=10, pady=10)

                passName_frame.pack(padx=10, pady=10)

                account_frame = tk.Frame(window,width=100)

                account_msg = tk.Label(
                    account_frame,
                    text=char_lib.INTERFACE.INPUT_ACCOUNT_LABLE.value,
                    font=("Arial", 15)
                )
                account_msg.pack(side=tk.LEFT, padx=10, pady=10)

                account_input_box = tk.Entry(
                    account_frame,
                    width=10,
                    font=("Arial", 15)
                )
                account_input_box.pack(side=tk.RIGHT, padx=10, pady=10)

                account_frame.pack(padx=10, pady=10)

                password_frame = tk.Frame(window, width=100)

                password_msg = tk.Label(
                    password_frame,
                    text=char_lib.INTERFACE.INPUT_PASSWORD_LABLE.value,
                    font=("Arial", 15)
                )
                password_msg.pack(side=tk.LEFT, padx=10, pady=10)

                password_input_box = tk.Entry(
                    password_frame,
                    width=10,
                    font=("Arial", 15),
                    show="*"
                )
                password_input_box.pack(side=tk.RIGHT, padx=10, pady=10)

                password_frame.pack(padx=10, pady=10)

                ok_button = tk.Button(
                    window,
                    text=char_lib.INTERFACE.OK_BUTTON.value,
                    font=("Arial", 15),
                    command=lambda: add_passbook_commands.add_pass_command(
                        passName=passName_input_box.get(),
                        account=account_input_box.get(),
                        password=password_input_box.get(),
                        window=window
                    )
                )
                ok_button.pack(padx=10, pady=10)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

class search_passbook_windows:

    """
    该类主要是搜索密码的交互窗口
    """

    @staticmethod
    def search_pass_window(window:tk.Toplevel):

        """
        该函数用于创建窗口，用于搜索密码，支持模糊查找
        """

        try:

            if loaded_data == None or loaded_data == {} or passbook_file_path == None or passbook_file_path == "":
                window.destroy()
                messagebox.showerror(char_lib.TITLE.ERROR_WINDOW.value, char_lib.INTERFACE.NOT_LOAD_PASSBOOK_TIP.value)
            else:
                input_frame = tk.Frame(window, width=100)

                input_box = tk.Entry(
                    input_frame,
                    width=30,
                    font=("Arial", 10)
                )
                input_box.pack(side=tk.LEFT, padx=10, pady=10)

                ok_button = tk.Button(
                    input_frame,
                    text=char_lib.INTERFACE.SEARCH_BUTTON_TOP.value,
                    font=("Arial", 10),
                    command=lambda: search_passbook_commands.search_pass_command(
                        passName=input_box.get(),
                        window=window,
                        text_area=text_area
                    )
                )
                ok_button.pack(side=tk.LEFT, padx=10, pady=10)

                input_frame.pack(padx=10, pady=10)

                text_area = scrolledtext.ScrolledText(
                    window,
                    wrap=tk.WORD,
                    width=50,
                    height=20,
                )
                text_area.pack(padx=10, pady=10)

                text_area.config(state=tk.DISABLED)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))



class MainCommands:

    """
    该类中包含了主界面的所有可交互部分的执行操作
    """

    @staticmethod
    def load_passbook():

        """
        该函数为加载密码簿的底层操作函数
        """

        try:
            file_path = filedialog.askopenfilename(
                title=char_lib.TITLE.CHOSE_LOAD_PASSBOOK_WINDOW.value,
                initialdir=os.path.expanduser("~"),
                filetypes=[config.PASSBOOK_FILE_EXTENSION, ("所有文件", "*.*")]
            )
            if file_path:
                global passbook_file_path
                passbook_file_path = file_path

                window = tk.Toplevel(root)
                window.title(char_lib.TITLE.EMAILECHECK_WINDOW.value)
                window.geometry(config.TOPLEVEL_WINDOW_SIZE)
                window.resizable(False, False)
                window.transient(root)

                check_email_windows.check_email_window(window=window)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

    @staticmethod
    def new_passbook():

        """
        该函数为创建新密码簿的底层操作函数
        """

        try:
            file_path = filedialog.asksaveasfilename(
                title=char_lib.TITLE.CHOSE_SAVE_PASSBOOK_WINDOW.value,
                initialdir=os.path.expanduser("~"),
                filetypes=[config.PASSBOOK_FILE_EXTENSION, ("所有文件", "*.*")]
            )
            if file_path:
                global passbook_file_path
                passbook_file_path = file_path

                window = tk.Toplevel(root)
                window.title(char_lib.TITLE.NEW_WINDOW.value)
                window.geometry(config.TOPLEVEL_WINDOW_SIZE)
                window.resizable(False, False)
                window.transient(root)

                new_passbook_windows.add_email_window(window=window)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

    @staticmethod
    def save_passbook():

        """
        该函数为保存密码簿的底层操作函数
        """

        try:
            if passbook_file_path == None or passbook_file_path == "":
                messagebox.showerror(char_lib.TITLE.ERROR_WINDOW.value, char_lib.INTERFACE.NOT_LOAD_PASSBOOK_TIP.value)
            else:
                if loaded_data[config.DATA_JSON_NAME] == None or loaded_data[config.DATA_JSON_NAME] == {}:
                    chose = messagebox.askyesno(char_lib.TITLE.TIP_WINDOW.value, char_lib.INTERFACE.NO_DATA_PASSBOOK_TIP.value)
                    if chose:
                        loaded_data[config.DATA_JSON_NAME] = {}
                        basic_function.save_data(Data=loaded_data, OutputFilePath=passbook_file_path)
                else:
                    basic_function.save_data(Data=loaded_data, OutputFilePath=passbook_file_path)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

    @staticmethod
    def search_passbook():

        """
        该函数为搜索密码簿的底层操作函数
        """

        try:
            window = tk.Toplevel(root)
            window.title(char_lib.TITLE.SEARCH_WINDOW.value)
            window.geometry(config.TOPLEVEL_WINDOW_SIZE)
            window.transient(root)
            window.resizable(False, False)

            search_passbook_windows.search_pass_window(window=window)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))

    @staticmethod
    def add_passbook():

        """
        该函数为添加密码簿的底层操作函数
        """

        try:
            window = tk.Toplevel(root)
            window.title(char_lib.TITLE.ADD_WINDOW.value)
            window.geometry(config.TOPLEVEL_WINDOW_SIZE)
            window.transient(root)
            window.resizable(False, False)

            add_passbook_windows.add_pass_window(window=window)
        except Exception as e:
            messagebox.showerror(char_lib.TITLE.SYSERROR_WINDOW.value, str(e))
            # root.quit()




root = tk.Tk()
root.title(char_lib.TITLE.MAIN_WINDOW.value)
root.geometry(config.MAIN_WINDOW_SIZE)
root.resizable(False, False)

menu_bar = tk.Menu(root)

file_menu = tk.Menu(menu_bar, tearoff=0)

file_menu.add_command(label=char_lib.MENU.LOAD_PASSBOOK.value, command=MainCommands.load_passbook, accelerator="Ctrl+L")
file_menu.add_command(label=char_lib.MENU.NEW_PASSBOOK.value, command=MainCommands.new_passbook, accelerator="Ctrl+N")
file_menu.add_command(label=char_lib.MENU.SAVE_PASSBOOK.value, command=MainCommands.save_passbook, accelerator="Ctrl+S")
file_menu.add_separator()
file_menu.add_command(label=char_lib.MENU.EXIT.value, command=on_closing, accelerator="Ctrl+Q")

menu_bar.add_cascade(label=char_lib.MENU.FILE.value, menu=file_menu)
root.config(menu=menu_bar)

status_label = tk.Label(
    root,
    text="",
    padx=10,
    pady=20,
    font=("Arial", 15)
)

status_label.pack(padx=10, pady=10)

search_button = tk.Button(
    root,
    text=char_lib.INTERFACE.SEARCH_BUTTON_MAIN.value,
    width=25,
    height=2,
    command=MainCommands.search_passbook
)

search_button.pack(padx=10, pady=10)

add_button = tk.Button(
    root,
    text=char_lib.INTERFACE.ADD_BUTTON.value,
    width=25,
    height=2,
    command=MainCommands.add_passbook
)

add_button.pack(padx=10, pady=10)

path_lable = tk.Label(
    root,
    text="",
    padx=10,
    pady=20,
    font=("Arial", 10)
)

path_lable.pack(side=tk.BOTTOM, padx=10, pady=10)

root.bind_all("<Control-l>", lambda event: MainCommands.load_passbook())
root.bind_all("<Control-n>", lambda event: MainCommands.new_passbook())
root.bind_all("<Control-s>", lambda event: MainCommands.save_passbook())
root.bind_all("<Control-q>", lambda event: on_closing())

check_loaded_data_thread_stop_event = threading.Event()
check_loaded_data_thread = threading.Thread(target=check_loaded_data)
check_loaded_data_thread.start()

root.protocol("WM_DELETE_WINDOW", on_closing)

root.mainloop()