"""
    -------------------------------------------------------------------------
    Author: JiangZongchen
    Email: hello2022_jzc@yeah.net
    Time: 2025.3
    FileName: __init__.py
    -------------------------------------------------------------------------

    该文件为模块的入口文件, 用于导入模块, 并对模块进行初始化

"""

from .FileOperation import FileOperation as FO

from .Email import *

import config

class FileOperation:
    def WriteFile(self, InputFilePath:str, OutputFilePath:str) -> None:
        return FO.WriteFile(InputFilePath, OutputFilePath)

    def GetData(self, InputFilePath:str) -> dict | None:
        return FO.GetData(InputFilePath)
    
    def GetKey(self, InputFilePath:str) -> int | None:
        return FO.GetKey(InputFilePath)
    
    def CheckEmail(self, InputEmail:str, InputFilePath:str) -> str | None:
        return FO.CheckEmail(InputEmail, InputFilePath)
    
class SendEmail:
    def SendVer(self, verCode:int, Email:str):
        return send_ver_code(
            ver_code=verCode,
            email=Email,
            email_sender=config.EMAIL_SENDER,
            email_pass=config.EMAIL_SENDER_PASS,
            email_server=config.EMAIL_SERVER,
            email_server_port=config.EMAIL_SERVER_PORT
        )