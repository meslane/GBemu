for opcode in range(256):
    nibble = [opcode & 0x0F, (opcode >> 4) & 0x0F]
    #insert if statement here:
    if ((nibble[1] >= 0x4 and nibble[1] <= 0x7) and (nibble[0] % 8 == 0x6) and (opcode != 0x76)):
        print("{:02x}".format(opcode), end=" ")
    else:
        print("..", end=" ")
    
    if ((opcode+1) % 16 == 0 and (opcode + 1 != 0)):
        print()