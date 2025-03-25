"""
    -------------------------------------------------------------------------
    Author: JiangZongchen
    Email: hello2022_jzc@yeah.net
    Time: 2025.3
    FileName: config.py
    -------------------------------------------------------------------------

    该文件为程序配置文件，用于对程序进行简单配置

    ATTENTION:
        除非你知道你在做什么，否则请不要随意修改该文件，极有可能让程序出现未知错误！

"""

import os

# 项目根目录，请勿修改，可能影响程序正常运行
ROOT = os.path.join(os.getcwd(),"src")

# 缓存目录
CACHE_PATH = os.path.join(ROOT,"cache")

# json文件中的变量名称，请注意要与C语言中的名称保持一致
ACCOUNT_JSON_NAME = "account"
PASSWORD_JSON_NAME = "password"

# json文件中的变量名称，请注意要与C语言中的名称保持一致
EMAIL_JSON_NAME = "email"
KEY_JSON_NAME = "key"
DATA_JSON_NAME = "data"

# 密码本文件扩展名
PASSBOOK_FILE_EXTENSION = ("密码本文件","*.pb")

# SMTP服务器配置
EMAIL_SENDER = "xxx@xxx.com" # 发件人邮箱
EMAIL_SENDER_PASS = "xxxx" # 发件人邮箱密码
EMAIL_SERVER = "smtp.xxx.com" # smtp服务器地址
EMAIL_SERVER_PORT = 25 # smtp服务器端口

# 窗口大小
MAIN_WINDOW_SIZE = "800x500"
TOPLEVEL_WINDOW_SIZE = "700x400"

# 程序语言，请注意需要在语言包中添加对应语言包
LANGUAGE = "zh_CN"