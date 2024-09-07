class ShiftReduceParser:

  def __init__(self, name_: str):
    self.srname: str = name_


class LR0Parser(ShiftReduceParser):

  def __init__(self, myname_: str, parentname_: str):
    self.lr0name: str = myname_
    ShiftReduceParser.__init__(self, parentname_)


class CLRParser(ShiftReduceParser):

  def __init__(self, myname_: str, parentname_: str):
    self.clrname: str = myname_
    ShiftReduceParser.__init__(self, parentname_)


class LALRParser(CLRParser):

  def __init__(self, myname_: str, clrname_: str, srname_: str):
    self.lalrname: str = myname_
    CLRParser.__init__(self, clrname_, srname_)

  def print_name(self):
    print("SLR name:")
    print(self.srname)
    print("CLR name:")
    print(self.clrname)
    print("LALR name:")
    print(self.lalrname)


def main():
  obj: LALRParser = LALRParser("LALR", "CLR", "Shift-Reduce")
  obj.print_name()


if __name__ == "__main__":
  main()
