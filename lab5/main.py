# -*- coding: utf-8 -*-

import sys
from os import walk
import smtplib

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders


def in_file(path: str, lookup: str) -> bool:
    with open(path, 'r') as f:
        data = str(f.read())
        return data.find(lookup) != -1


def explore_txt(path: str):
    res = []
    filenames = next(walk(path), (None, None, []))[2]

    for file in filenames:
        if file.split('.')[-1] == 'txt':
            res.append((path + '/' + file, file))
    return res


def new_attachment(path: str, name: str):
    attachment = MIMEBase('application', "octet-stream")
    with open(path, "rb") as f:
        tempData = f.read()

    attachment.set_payload(tempData)
    encoders.encode_base64(attachment)
    attachment.add_header('Content-Disposition', 'attachment; filename="%s"' % name)
    return attachment


def get_attachments(lookup: str):
    att_arr = []

    for path, file in explore_txt('./static'):
        if not in_file(path, lookup):
            continue
        att_arr.append(new_attachment(path, file))
        print('%s attached to email' % file)
    return att_arr


def new_mail(from_addr: str, to_addr: str, subj="Test", text="Test"):
    mail = MIMEMultipart()
    mail['From'] = from_addr
    mail['To'] = to_addr
    mail['Subject'] = subj
    mail.attach(MIMEText(text))
    return mail


def send_gmail(mail: MIMEBase, pw: str):
    HOST = "smtp.gmail.com"
    PORT = 587

    server = smtplib.SMTP(HOST, PORT)
    print('connection to SMTP server established')
    server.ehlo()
    server.starttls()
    server.login(mail['From'], pw)
    server.sendmail(mail['From'], [mail['To']], mail.as_string())
    server.quit()


def main(args):
    to_addr = args[0]
    from_addr = args[1]
    password = args[2]
    lookup = args[3]

    mail = new_mail(from_addr, to_addr, *args[4:6])

    for att in get_attachments(lookup):
        mail.attach(att)

    send_gmail(mail, password)
    print('Email sent')


if __name__ == '__main__':
    print('Cmd arguments format:\tDest address, Src address, Password, Search, *Title, *Text')
    if len(sys.argv) < 5:
        print("Not enough arguments")
    else:
        main(sys.argv[1:])
