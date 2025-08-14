import json
import os

def json_to_txt():
    json_files = [
        'eco.json/ecoA.json',
        'eco.json/ecoB.json',
        'eco.json/ecoC.json',
        'eco.json/ecoD.json',
        'eco.json/ecoE.json',
        'eco.json/eco_interpolated.json'
    ]
    output_txt_file = 'opening/opening_book.txt'

    with open(output_txt_file, 'w', encoding='utf-8') as txt_file:
        for json_file in json_files:
            if os.path.exists(json_file):
                with open(json_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    if isinstance(data, dict):
                        for entry in data.values():
                            if isinstance(entry, dict):
                                # Correct key is 'moves', not 'pgn'
                                pgn = entry.get('moves', '')
                                name = entry.get('name', '')
                                if pgn and name:
                                    sanitized_name = name.replace('"', '\"')
                                    txt_file.write(f"{pgn} # {sanitized_name}\n")

    print(f"Successfully created {output_txt_file}")

def txt_to_cpp():
    input_txt_file = 'opening/opening_book.txt'
    output_cpp_header = 'opening/opening_book.h'

    with open(input_txt_file, 'r', encoding='utf-8') as txt_file:
        lines = txt_file.readlines()

    with open(output_cpp_header, 'w', encoding='utf-8') as h_file:
        h_file.write("#ifndef OPENING_BOOK_H\n")
        h_file.write("#define OPENING_BOOK_H\n\n")
        h_file.write("#include <string>\n")
        h_file.write("#include <map>\n\n")
        h_file.write("namespace {\n")
        h_file.write("const std::map<std::string, std::string> opening_book = {\n")

        for line in lines:
            line = line.strip()
            if not line or '#' not in line:
                continue
            
            parts = line.split('#', 1)
            pgn = parts[0].strip()
            name = parts[1].strip()
            
            line_to_write = '    {"' + pgn + '", "' + name + '"},\n'
            h_file.write(line_to_write)

        h_file.write("};\n")
        h_file.write("} // anonymous namespace\n\n")
        h_file.write("#endif // OPENING_BOOK_H\n")
    
    print(f"Successfully created {output_cpp_header}")

if __name__ == "__main__":
    json_to_txt()
    txt_to_cpp()