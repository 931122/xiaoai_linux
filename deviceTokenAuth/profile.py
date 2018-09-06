#!/usr/bin/python
# -*- coding:utf-8 -*-
import base64
import hashlib
from ecdsa import SigningKey, VerifyingKey
from ecdsa.util import sigencode_der, sigdecode_der
import wget
import sys, getopt, os
def generate_signature(app_id, device_id, privatekey):
    """
      生成签名
    """
    private_key = SigningKey.from_pem(open(privatekey).read())
    signature = base64.urlsafe_b64encode(private_key.sign(device_id, hashfunc=hashlib.sha256, sigencode=sigencode_der))
    return signature


def verify_signature(app_id, device_id, signature, publickey):
    """
      验证签名 服务端实现
    """
    public_key = VerifyingKey.from_pem(open(publickey).read())
    check_res = public_key.verify(base64.urlsafe_b64decode(signature), device_id,
                                  hashfunc=hashlib.sha256, sigdecode=sigdecode_der)
    return check_res

def write_to_file(context, file_name):
  with open(file_name,'w') as f: # 如果filename不存在会自动创建， 'w'表示写数据，写之前会清空文件中的原有数据！
    f.write(context)

def main(argv):
  app_id = ''
  device_id = ''
  profile = ''
  error_msg = 'profile.py -a <app_id> -d <device_id> -p <profile> -P <publickey> -K <privatekey>'
  try:
    opts, args = getopt.getopt(argv,"ha:d:p:P:K:")
  except getopt.GetoptError:
    print error_msg
    print "Parse parms error"
    sys.exit(2)

  if (len(opts) <= 1 or len(opts) >= 6):
    print error_msg
    sys.exit(1)
  
  index = 0
  opt, arg = opts[index]
  if (opt != '-a'):
    print error_msg
    sys.exit(1)
  app_id = arg
  index = index + 1
  opt, arg = opts[index]
  if (opt != '-d'):
    print error_msg
    sys.exit(1)
  device_id = arg
  index = index + 1
  opt, arg = opts[index]
  if (opt != '-p'):
    print error_msg
    sys.exit(1)   
  profile = arg
  index = index + 1
  opt, arg = opts[index]
  if (opt != '-P'):
    print error_msg
    print "Get public key failed"
    sys.exit(1)
  publickey = arg
  index = index + 1
  opt, arg = opts[index]
  if (opt != '-K'):
    print error_msg
    print "Get private key failed"
    sys.exit(1)
  privatekey = arg
  index = index + 1
  print 'app_id:' + app_id
  print 'device_id：' + device_id
  print 'base64 deviceid:' + base64.b64encode(device_id)
  print 'profile：' + profile
  print 'publickey: ' + publickey
  print 'privatekey: ' + privatekey
  sign_res = generate_signature(app_id, device_id, privatekey)
  print sign_res
  print "\n"
  res = verify_signature(app_id, device_id, sign_res, publickey)
  print res
  print "\n"
  write_to_file(sign_res, profile)
  
  '''
  PROFILR_URL "https://account.ai.xiaomi.com/thirdparty/auth/token"
  REFRESH_TOKEN_URL "https://account.ai.xiaomi.com/thirdparty/refresh/token"
   
  '''
  params ='client_id='+app_id+'&device='+base64.b64encode(device_id)+'&sign='+sign_res
  #print 'request param:  ' + params 
  curlcmd = 'curl -d  "'+ params +'"' + ' "https://account.ai.xiaomi.com/thirdparty/auth/token"'
  print curlcmd
  print '\n'
  print '\n'
  os.system(curlcmd)
  print '\n'
  print '\n'
  
  
if __name__ == "__main__":
   main(sys.argv[1:])
