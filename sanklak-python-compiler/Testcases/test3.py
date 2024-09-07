class A:
	def __init__(self, y: int, z: bool, name: str):
		self.y: int = y
		self.z: bool = z
		self.name: str = name

	# setters
	def set_y(self, y: int) -> None:
		self.y = y

	def set_z(self, z: bool) -> None:
		self.z = z

	def set_name(self, name: str) -> None:
		self.name = name

	# getters
	def get_y(self) -> int:
		return self.y
	
	def get_z(self) -> bool:
		return self.z

	def get_name(self) -> str:
		return self.name

	# printers
	def print_y(self) -> None:
		print(self.y)

	def print_z(self) -> None:
		print(self.z)
	
	def print_name(self) -> None:
		print(self.name)

	def print_all(self) -> None:
		print(self.y)
		print(self.z)
		print(self.name)

def print_obj(obj: A) -> None:
	print("printing using printers\n")
	obj.print_y()
	obj.print_z()
	obj.print_name()

	y: int
	z: bool
	name: str

	print("printing using getters\n")
	y = obj.get_y()
	z = obj.get_z()
	name = obj.get_name()
	print(y)
	print(z)
	print(name)

def main():
	y: int
	z: bool
	name: str

	obj: A = A(6, True, "obj\n")

	print_obj(obj)

	print("using setters to change values\n")
	y = 60
	z = False
	name = "new_obj\n"
	obj.set_y(y)
	obj.set_z(z)
	obj.set_name(name)
	
	print_obj(obj)

if __name__ == "__main__":
    main()
