# PassBookTool

## 免责声明及项目声明：
- 该项目仅供学习交流，请勿用于非法用途！
- 项目仅提供可行思路，若因不当使用导致的财产损失/数据泄露，与项目作者/参与项目的开发者无关！
- 所有代码文件中开头部分的作者声明，仅用于标识，请勿删除！
- 项目中的算法若出现雷同，纯属巧合，请联系作者删除（hello2022_jzc@yeah.net）
- 请遵循开源协定，如侵权，必将追究法律责任！


## 项目介绍：
该项目是一个用于本地化保存密码的工具，通过对密码和账户进行加密，然后将数据保存到本地，当需要查询密码的时候，只需将生成的.pb文件打开并进行验证即可

## 目录结构：

```
PassBookTool
├── API.py
├── cache
├── CharacterLibs
│   ├── en_us.json
│   ├── __init__.py
│   └── zh_cn.json
├── config.py
├── Encryption
│   ├── __init__.py
│   └── RSAEnc
│       ├── RSA.c
│       └── setup.py
├── LICENSE
├── main.py
├── Module
│   ├── Email.py
│   ├── FileOperation
│   │   ├── FileOperation.c
│   │   └── setup.py
│   └── __init__.py
├── README.md
└── requirements.txt
```

## 使用方法

### 0. 确认一些事情：
- 确认Python已经安装，且版本>=3.10
- 确认电脑安装了gcc编译器
- 电脑安装了json-c库，具体安装方法请自行查询
- 确认可以正常通过venv创建Python虚拟环境
- 需要一个稳定的网络连接

### 1. 创建Python虚拟环境并启动虚拟环境：
选择一个你喜欢的目录，通过git命令克隆本仓库 <br>
此时你或许会看到一个名称为PassBookTool的目录，请将该目录更名为src <br>
此时在src父目录下执行以下命令创建虚拟环境：


```bash
python3 -m venv PassBookTool
```

上述命令将会在当前目录下创建一个名为PassBookTool的虚拟环境 <br>
此时可以将克隆下来修改名称后的src目录移动到PassBookTool目录下 <br>
执行成功后启动虚拟环境，进入PassBookTool目录后执行命令：<br>
```bash
# Linux/MacOS:
source bin/activate

# Windows:
Scripts\activate
```


### 2. 安装依赖
如果上述步骤执行成功了，那已经完成大多数的准备工作了，接下来只需要安装依赖即可 <br>
进入PassBookTool/src目录，执行以下命令安装依赖：
```bash
pip install -r requirements.txt
```

### 3. 配置程序
配置文件在PassBookTool/src/config.py中，你需要修改以下部分：
```python
# SMTP服务器配置
EMAIL_SENDER = "xxx@xxx.com" # 发件人邮箱
EMAIL_SENDER_PASS = "xxxx" # 发件人邮箱密码
EMAIL_SERVER = "smtp.xxx.com" # smtp服务器地址
EMAIL_SERVER_PORT = 25 # smtp服务器端口
```
上述配置为SMTP服务器配置，如果你没有独立的smtp服务器，可以打开个人邮箱的smtp服务，其中邮箱密码部分为生成的专有密钥，具体请自行搜索 <br>
若需要详细教程，请提Issues或通过邮箱联系我 <br>

### 4. 对C语言部分进行编译：

如果上面的步骤你都完成了，那已经完成大部分步骤了 <br>
这时候需要编译RSA加密算法，请进入如下目录：

> PassBookTool/src/Encryption/RSAEnc

进入该目录后，执行命令：
```bash
python3 setup.py build_ext --inplace
```
这会将c语言编译成为python模块 <br><br>

按照相同的办法，进入如下目录：

> PassBookTool/src/Module/FileOperation

然后执行相同的命令
```bash
python3 setup.py build_ext --inplace
```
### 5. 启动应用
进入 PassBookTool/src 目录下，执行如下命令：
```bash
python3 main.py
```
如果没有任何错误，看见了一个窗口，那么恭喜你，成功的运行了这个程序


## 联合开发：
  该项目欢迎大家参与，如果大家有更好的想法，欢迎大家提出贡献，作者会及时合并处理
  >### E-MAIL:
  >hello2022_jzc@yeah.net  
  >### 贡献者：
  >* [jason-jzc](https://github.com/jason-jzc)