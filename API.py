"""
    -------------------------------------------------------------------------
    Author: JiangZongchen
    Email: hello2022_jzc@yeah.net
    Time: 2025.3
    FileName: API.py
    -------------------------------------------------------------------------

    该文件为接口文件，用于将单一功能结合为复杂功能块，使得前端调用更加简洁。

"""

import random
import os
import json

import Module
import Encryption
import config

import copy

class Basic_Function:
    def __init__(self):
        self.EmailVerSend = Module.SendEmail() # 初始化发送邮件模块
        self.FO = Module.FileOperation() # 初始化文件操作模块
        self.Enc = Encryption.RSAEncryption() # 初始化加密模块

        self.email = None # 邮箱
    
    def check_email(self, InputFilePath:str, InputEmail:str) -> bool:
        """
        该函数用于验证输入的邮箱是否正确
        若邮箱正确，则向邮箱发送验证码，并返回True
        否则返回False
        """

        if self.FO.CheckEmail(InputEmail=InputEmail, InputFilePath=InputFilePath) == InputEmail: # 判断邮箱是否正确
            self.email = InputEmail # 将邮箱赋值给self.email
            self.vercode = str(random.randint(1000,9999)) # 生成验证码
            self.EmailVerSend.SendVer(verCode=self.vercode, Email=InputEmail) # 发送验证码
            return True
        else:
            return False

    def get_data(self, InputFilePath:str, VerifyCode:str) -> dict | None:
        """
        该函数用于获取文件数据，并解密，同时会对验证码进行校验
        若验证码正确，则返回一个字典，否这返回None
        """
        data = {}
        if VerifyCode == self.vercode: # 判断验证码是否正确
            Key = self.FO.GetKey(InputFilePath=InputFilePath) # 若正确则获取密钥
            PassData = self.FO.GetData(InputFilePath=InputFilePath) # 获取加密数据
            if (not Key == None) and (not PassData == None): # 判断上述函数是否执行成功
                data[config.EMAIL_JSON_NAME] = self.email # 将邮箱添加到返回数据中
                data[config.KEY_JSON_NAME] = None # 不会将密钥添加到返回数据中，保证数据安全
                for i in PassData.keys(): # 遍历加密数据，并进行解密
                    PassData[i][config.ACCOUNT_JSON_NAME] = self.Enc.decrypt(encryptedData=PassData[i][config.ACCOUNT_JSON_NAME], privateKey=Key) # 解密账号数据
                    PassData[i][config.PASSWORD_JSON_NAME] = self.Enc.decrypt(encryptedData=PassData[i][config.PASSWORD_JSON_NAME], privateKey=Key) # 解密密码数据
                data[config.DATA_JSON_NAME] = PassData # 将解密后的数据添加到返回数据中
                return data # 返回数据
            else:
                return None
        else:
            return None
    
    def new_passbook_data(self, Email:str) -> dict:
        """
        该函数用于生成新的密码簿数据
        返回一个字典
        字典格式与get_data函数返回格式相同，但不包含密钥、账户、密码数据，仅包含邮箱数据
        """
        data = {
            config.EMAIL_JSON_NAME: Email,
            config.KEY_JSON_NAME: None,
            config.DATA_JSON_NAME: None
        }
        return data
    
    def search_data(self, Data:dict, Search:str) -> dict | None:
        """
        该函数用于搜索密码簿数据
        返回一个字典，字典中为包含Search关键字的所有数据
        若搜索失败，则返回None
        其中
        Search为搜索的关键字
        Data为密码簿数据，即get_data函数返回的数据
        """
        try:
            passdata = Data[config.DATA_JSON_NAME]
            redata = {}
            for i in passdata.keys():
                if Search in i:
                    redata[i] = passdata[i]
        except:
            return None
        return redata
    
    def save_data(self, Data:dict, OutputFilePath:str) -> None:
        """
        该函数用于将原始数据加密后保存到指定路径
        """

        temp_data = copy.deepcopy(Data)

        publicKey = self.Enc.getPublicKey() # 生成公钥
        privateKey = self.Enc.getPrivateKey(PublicKey=publicKey) # 生成私钥

        temp_data[config.KEY_JSON_NAME] = privateKey # 将私钥添加到数据中

        # Data = copy.deepcopy(temp_data)

        passdata = temp_data[config.DATA_JSON_NAME] # 获取密码簿数据
        # passdata = sourcepassdata

        for i in passdata.keys(): # 遍历密码簿数据，并进行加密
            passdata[i][config.ACCOUNT_JSON_NAME] = self.Enc.encrypt(sourceData=passdata[i][config.ACCOUNT_JSON_NAME], publicKey=publicKey)
            passdata[i][config.PASSWORD_JSON_NAME] = self.Enc.encrypt(sourceData=passdata[i][config.PASSWORD_JSON_NAME], publicKey=publicKey)
        
        temp_data[config.DATA_JSON_NAME] = passdata # 将加密后的数据添加到数据中

        tempfile = os.path.join(config.CACHE_PATH, os.path.splitext(os.path.basename(OutputFilePath))[0] + ".json") # 创建临时文件
        
        with open(tempfile,"w+") as temp: # 将数据写入临时文件
            json.dump(temp_data, temp, ensure_ascii=False)

        self.FO.WriteFile(InputFilePath=tempfile, OutputFilePath=OutputFilePath) # 将临时文件写入目标文件

        # Data[config.DATA_JSON_NAME] = sourcepassdata

        return None
    
    def add_data(self, Data:dict, Account:str, Password:str, Name:str) -> dict | None:
        """
        该函数用于向密码簿中添加数据
        若密码名称不存在且添加成功则返回添加后的数据，否则返回None
        """
        passdata = Data[config.DATA_JSON_NAME]

        if passdata == None:
            passdata = {}

        if not Name in passdata.keys():

            passdata[Name] = {
                config.ACCOUNT_JSON_NAME: Account,
                config.PASSWORD_JSON_NAME: Password
            }

            Data[config.DATA_JSON_NAME] = passdata

            return Data
        else:
            return None
        
    def change_data(self, Data:dict, Name:str, Account:str, Password:str):
        """
        该函数用于修改密码簿中的数据
        若密码名称存在且修改成功则返回修改后的数据，否则返回None
        """

        passdata = Data[config.DATA_JSON_NAME]

        if Name in passdata.keys():

            passdata[Name] = {
                config.ACCOUNT_JSON_NAME: Account,
                config.PASSWORD_JSON_NAME: Password
            }

            Data[config.DATA_JSON_NAME] = passdata

            return Data
        else:
            return None