from io import BufferedWriter
import random

def randomLenString()->str:
    currentChar:str = "k"
    returnStr:str = ""
    randomMinLen:int = random.randint(100,1500)
    while ord(currentChar) != 40 or len(returnStr)<randomMinLen:
        currentChar = chr(random.randint(40,126))
        if ord(currentChar) == 92:
            pass
        else:
            returnStr += currentChar
    return returnStr


def writeTestMetod(currentMetod:int,metodName:str,fileHandle:BufferedWriter)->None:
    fileHandle.write(f"TEST_METHOD({metodName}) \n{'{'}\n".encode("ascii"))
    randomStr:str = randomLenString()
    if(currentMetod!=1):
        splitLimit:int = random.randint(-1,60)
        charToSplitOn:str = chr(random.randint(40,126))
        if ord(charToSplitOn) == 92:
            charToSplitOn = "."
    if(currentMetod!=0):
        charToStripOn:str = chr(random.randint(40,126))
        if ord(charToStripOn) == 92:
            charToStripOn = "."
    fileHandle.write(f"String currentStr = stringCreate(\"{randomStr}\",{len(randomStr)});\n".encode("ascii"))
    fileHandle.write(f"if(currentStr.listType == ListNone)\n Assert::IsTrue(false);\n".encode("ascii"))
    match(currentMetod):
        case 0:
            correctStr:list[str]
            splitDir:int = random.randint(0,1)
            if splitDir == True and splitLimit != -1:
                correctStr = randomStr.rsplit(charToSplitOn,splitLimit)
                fileHandle.write(f"String splitStr = stringSplit(&currentStr,'{charToSplitOn}',true,{-splitLimit});".encode("ascii"))
                fileHandle.write("\nfor(size_t i = 0; i < splitStr.amountOfElements;i++)\nstringAdd(reinterpret_cast<String*>(arrayListElementGetGeneric(&splitStr,i)),\"\\0\",1);".encode("ascii"))
                iterator:int = 0
                for i in range(len(correctStr)-1,-1,-1):
                    if (len(correctStr[i]) == 0):
                        continue
                    fileHandle.write(
                        f"\nAssert::AreEqual(\"{correctStr[i]}\",reinterpret_cast<char*>(reinterpret_cast<String*>(arrayListElementGetGeneric(&splitStr,{iterator}))->list));\n".encode(
                            "ascii"))
                    iterator+=1
            else:
                correctStr = randomStr.split(charToSplitOn,splitLimit)
                fileHandle.write(f"String splitStr = stringSplit(&currentStr,'{charToSplitOn}',{splitLimit!=-1},{splitLimit});".encode("ascii"))
                fileHandle.write("\nfor(size_t i = 0; i < splitStr.amountOfElements;i++)\nstringAdd(reinterpret_cast<String*>(arrayListElementGetGeneric(&splitStr,i)),\"\\0\",1);".encode("ascii"))
                iterator:int = 0
                for i in range(len(correctStr)):
                    if (len(correctStr[i]) == 0):
                        continue
                    fileHandle.write(
                        f"\nAssert::AreEqual(\"{correctStr[i]}\",reinterpret_cast<char*>(reinterpret_cast<String*>(arrayListElementGetGeneric(&splitStr,{iterator}))->list));\n".encode(
                            "ascii"))
                    iterator+=1
        case 1:
            correctStr:str = ""
            correctBeforeStr:list[str] = randomStr.split(charToStripOn)
            for element in correctBeforeStr:
                correctStr += element
            fileHandle.write(f"String stripedStr = stringStrip(&currentStr,'{charToStripOn}',false,0);\n".encode("ascii"))
            fileHandle.write("stringAdd(reinterpret_cast<String*>(&stripedStr),\"\\0\",1);".encode("ascii"))
            fileHandle.write(f"\nAssert::AreEqual(\"{correctStr}\",reinterpret_cast<char*>(stripedStr.list));\n".encode("ascii"))
        
        case 2:
            correctStr:str|list[str] = ""
            correctBeforeStr:list[str] = randomStr.split(charToStripOn)
            for element in correctBeforeStr:
                correctStr += element
            fileHandle.write(f"String stripedStr = stringStrip(&currentStr,'{charToStripOn}',false,0);\n".encode("ascii"))
            splitDir: int = random.randint(0, 1)
            if splitDir == True and splitLimit != -1:
                correctStr = correctStr.rsplit(charToSplitOn, splitLimit)
                fileHandle.write(f"String splitStr = stringSplit(&stripedStr,'{charToSplitOn}',true,{-splitLimit});".encode("ascii"))
                fileHandle.write("\nfor(size_t i = 0; i < splitStr.amountOfElements;i++)\nstringAdd(reinterpret_cast<String*>(arrayListElementGetGeneric(&splitStr,i)),\"\\0\",1);".encode("ascii"))
                iterator: int = 0
                for i in range(len(correctStr) - 1, -1,-1):
                    if (len(correctStr[i]) == 0):
                        continue
                    fileHandle.write(
                        f"\nAssert::AreEqual(\"{correctStr[i]}\",reinterpret_cast<char*>(reinterpret_cast<String*>(arrayListElementGetGeneric(&splitStr,{iterator}))->list));\n".encode(
                            "ascii"))
                    iterator += 1
            else:
                correctStr = correctStr.split(charToSplitOn, splitLimit)
                fileHandle.write(
                    f"String splitStr = stringSplit(&stripedStr,'{charToSplitOn}',{splitLimit != -1},{splitLimit});\n".encode(
                        "ascii"))
                fileHandle.write(
                    "\nfor(size_t i = 0; i < splitStr.amountOfElements;i++)\nstringAdd(reinterpret_cast<String*>(arrayListElementGetGeneric(&splitStr,i)),\"\\0\",1);".encode(
                        "ascii"))
                iterator:int = 0
                for i in range(len(correctStr)):
                    if (len(correctStr[i]) == 0):
                        continue
                    fileHandle.write(f"\nAssert::AreEqual(\"{correctStr[i]}\",reinterpret_cast<char*>(reinterpret_cast<String*>(arrayListElementGetGeneric(&splitStr,{iterator}))->list));\n".encode("ascii"))
                    iterator+=1
        case _:
            pass
    fileHandle.write("}\n".encode("ascii"))

with open("./current.cpp","ab") as file:
    for j in range(3):
        file.write(f"TEST_CLASS(StringTest{j}){'{'}\n".encode("ascii"))
        for i in range(500):
            writeTestMetod(j,f"Metod{i}",file)
        file.write("};\n".encode("ascii"))
