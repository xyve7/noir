
bitmap = [0, 0, 0, 0, 0, 0, 0, 0]

def alloc(count: int) -> int:
    found = 0
    start = 0
    for i in range(0, len(bitmap)):
        if bitmap[i] == 1:
            found = 0
        else:
            if found == 0:
                start = i
            found += 1
        
        if found == count:
            for j in range(0, count):
                bitmap[start + j] = 1;
            
            return start

    return 1000


def main():
    print(bitmap)

    print(alloc(2))
    print(alloc(3))
    print(alloc(1))
    print(alloc(1))

    print(bitmap)

if __name__ == "__main__":
    main()
