class Animal:
    def __init__(self, name: str, species: str, age: int):
        self.name:str = name
        self.species:str = species
        self.age:int = age

class Pet(Animal):
    def __init__(self, name: str, species: str, age: int, owner: str):
        Animal.__init__(self, name, species, age)
        self.owner:str = owner

def main() -> None:
    a1:Pet = Pet("Buddy", "Dog", 3, "Alice")
    a2:Pet = Pet("Whiskers", "Cat", 5, "Bob")
    a3:Pet = Pet("Goldie", "Fish", 1, "Charlie")
    a4:Pet = Pet("Polly", "Parrot", 7, "David")
    pets: list[Pet] = [a1,a2,a3,a4]

    oldest_pet: Pet = pets[0]
    youngest_pet: Pet = pets[0]

    i:int = 0
    length:int = len(pets)
    for i in range(length):
        pet:Pet = pets[i]
        if pet.age > oldest_pet.age:
            oldest_pet = pet
        if pet.age < youngest_pet.age:
            youngest_pet = pet

    print("The oldest pet is: "),
    print(oldest_pet.name)
    print(" -> ")
    print(oldest_pet.species)
    print(", Age: ")
    print(oldest_pet.age)
    print("The youngest pet is: ")
    print(youngest_pet.name)
    print(" -> ")
    print(youngest_pet.species)
    print(", Age: ")
    print(youngest_pet.age)

if __name__ == "__main__":
    main()
