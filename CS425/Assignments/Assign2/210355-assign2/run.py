import random

def add_error(message):
  n = len(message)

  bit = random.choice(range(n))

  msg = list(message)
  if(msg[bit]=='1'):
    msg[bit]= '0'
  else:
    msg[bit] = '1'

  return ''.join(msg)

def generate_msg(k):
  msg = ""

  for _ in range(k):
    bit = random.choice([0,1])

    if bit:
      msg += '1'
    else:
      msg += '0'

  return msg


def get_xor(dividend_, divisor, offset):
  x = len(divisor)
  dividend = list(dividend_)

  for i in range(x):
    if(dividend[offset+i]==divisor[i]):
      dividend[offset+i] = '0'
    else:
      dividend[offset+i] = '1'

  dividend_new = ''.join(dividend)
  return dividend_new

def get_last( string ):
  str1 = list(string)
  for i in range(len(str1)):
    if(str1[i]=='1'):
      ret = ''.join(str1[i:])
      return ret
  return "0"

def get_remainder(dividend, divisor):
  org_msg = dividend
  quotient = ""
  k = len(dividend)
  n = len(divisor)+ k -1

  for _ in range(n-k):
    dividend += '0'

  for i in range(k):
    if(dividend[i]=='0'):
      quotient += '0'
      continue
    quotient += '1'
    dividend = get_xor(dividend,divisor,i)

  remainder = get_last(dividend)
  return remainder, quotient

def custom_check():

  k = input("Enter number of bits you want to send: ")

  data_block = generate_msg(int(k))
  print(f"generated Message to be send: {data_block}")

  crc_p = input("Enter the CRC pattern: ")
  # crc_p = "110101"
  r,q = get_remainder(data_block,crc_p)
  msg_T = data_block + r

  print(f"Message ready for transmission : {msg_T}")

  rcv_msg = add_error(msg_T)

  print(f"Message received : {rcv_msg}")

  r,q = get_remainder(rcv_msg,crc_p)

  if not(r=="0"):
    print("Received message has errors")
  else:
    print("Received message is error-free")

def default_check():
  data_block = input("Enter the data to be transmitted: ")
  print(f"you entered {len(data_block)}-bits data: {data_block}")

  crc_p = input("Enter the CRC pattern: ")
  print(f"you entered {len(crc_p)}-bits pattern: {crc_p}")

  remainder, quotient = get_remainder(data_block,crc_p)
  print(f"Final message to send is  {data_block + remainder }")
  # print(f"Final quotient left is  {quotient}")

def get_choice():
    print("--------------------Program Started---------------------")
    print("[1] Generate n-bits CRC message for transmission")
    print("[2] Generate random k-bits msg and check the  CRC on received end")


    choice =  input("Enter your choice [1]/[2]: ")
    return choice

choice = get_choice()

if choice == "1":
    default_check()

elif choice == "2":
    custom_check()
else:
  print("Invalid input")


