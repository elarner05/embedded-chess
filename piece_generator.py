from PIL import Image

def get_C_code_from_chess_piece(filename):
    pic = Image.open(filename)
    pic = pic.convert("RGBA")
    code = "void drawBishop(uint16_t x, uint16_t y) {\n"

    for y in range(pic.height):
        for x in range(pic.width):
            R, G, B, A = pic.getpixel((x, y))
            if A <  50:
                pass
            elif R < 100 and G < 100 and B < 100:
                code+=f"  tft.drawPixel(x+{x}, y+{y}, BLACK);\n"
            elif R > 150 and G > 150 and B > 150:
                code+=f"  tft.drawPixel(x+{x}, y+{y}, WHITE);\n"
        code+="\n"
    code = code[:-1]
    code+="}"
    return code

def capture_overlay_in_C(filename="capture_underlay.png"):
    pic = Image.open(filename)
    pic = pic.convert("RGBA")
    code = "int capture_pic[30][30] = {"

    for y in range(pic.height):
        current = "                           {"
        for x in range(pic.width):
            R, G, B, A = pic.getpixel((x, y))
            if A <  50:
                pass
            elif R < 100 and G < 100 and B < 100:
                current+="1, "
            elif R > 150 and G > 150 and B > 150:
                current+="0, "
        current = current[:-2]
        current+="},\n"
        code += current
    code = code[:-2]
    code+="}"
    return code


#print(get_C_code_from_chess_piece(filename="_wB.png"))
print(capture_overlay_in_C(filename="capture_underlay.png"))