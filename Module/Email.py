# #!/usr/bin/python
# # -*- coding: UTF-8 -*-
 
import smtplib
from email.mime.text import MIMEText
from email.utils import formataddr

import time

def send_ver_code(email:str, ver_code:int, email_sender:str, email_pass:str, email_server:str, email_server_port:int):
    mail_msg = f"""
    <h1>
    欢迎使用密码管理工具，您的验证码为：
    </h1>
    <h2>
    {ver_code}
    </h2>
    """
    msg = MIMEText(mail_msg, 'html', "utf-8")
    msg['From'] = formataddr(["PassTool",email_sender])
    msg['To'] = formataddr(["User",email])
    msg['Subject']=f"密码管理工具邮箱验证({time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())})"

    server = smtplib.SMTP_SSL(email_server, email_server_port)
    # server = smtplib.SMTP()
    # server.connect(email_server, email_server_port)
    server.login(email_sender, email_pass)
    server.sendmail(email_sender, [email, ], msg.as_string())
    server.quit()