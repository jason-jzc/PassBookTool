"""
    -------------------------------------------------------------------------
    Author: JiangZongchen
    Email: hello2022_jzc@yeah.net
    Time: 2025.3
    FileName: __init__.py
    -------------------------------------------------------------------------

    该文件为加密算法的API封装文件
    每个算法被封装为一个类，类名与算法名相同，类方法名与算法方法名相同

"""

from .RSAEnc import RSA

class RSAEncryption:
    def encrypt(self,sourceData:str, publicKey:int) -> list:
        """
        该函数用于加密数据
        :param sourceData: 待加密数据
        :param publicKey: 公钥
        :return: 加密后的数据
        """
        return RSA.encrypt(sourceData, publicKey)
    
    def decrypt(self,encryptedData:list, privateKey:int) -> str | None:
        """
        该函数用于解密数据
        :param encryptedData: 待解密数据
        :param privateKey: 私钥
        :return: 解密后的数据
        """
        return RSA.decrypt(encryptedData, privateKey)

    def getPublicKey(self) -> int:
        """
        该函数用于获取公钥
        :return: 公钥
        """
        return RSA.getPublicKey()
    
    def getPrivateKey(self,PublicKey:int) -> int:
        """
        该函数用于获取私钥
        :param PublicKey: 公钥
        :return: 私钥
        """
        return RSA.getPrivateKey(PublicKey)

    def Info(self) -> None:
        """
        该函数用于输出加密算法的欢迎信息
        """
        return RSA.Info()

    def clean(self) -> None:
        """
        该函数用于清除加密算法的缓存
        建议每次加密/解密后调用以释放内存, 防止内存泄漏
        """
        return RSA.clean()