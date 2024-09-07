def main():
    a:int  = 5
    b:int = 9
    c:float = 8.0
    d:float = 7.8
    c = c + a
    a += c
    c *= a
    c -= d
    a /= d 


















#class LALRParser:
#
#  def __init__(self, myname_: str, clrname_: str):
#    self.lalrname: str = myname_
#    self.clrname: str = clrname_
#
#
##class CHILDParser(LALRParser):
#class CHILDParser:
#
#  def __init__(self, myname_: str, clrname_: str, srname_: str):
#    self.srname : str = srname_
#    LALRParser.__init__(self,myname_,clrname_)
#
#  def print_name(self) -> None:
#    print("SLR name:")
#    print(self.srname)
#    print("CLR name:")
#    print(self.clrname)
#    print("LALR name:")
#    print(self.lalrname)
#
#def main():
#  obj: CHILDParser = CHILDParser("text_LALR", "text_CLR", "text_Shift-Reduce")
#  obj.print_name()
#  #  obj: LALRParser = LALRParser("text_myname","text_clr")
#
#
#if __name__ == "__main__":
#  main()
