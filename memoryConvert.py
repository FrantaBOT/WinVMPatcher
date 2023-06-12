import argparse
import os
import pefile

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("diff_file", help="Path to the diff file")
    parser.add_argument("binary_file", help="Path to the binary file")
    args = parser.parse_args()

    if not os.path.isfile(args.diff_file):
        print("Diff file not found")
        exit(1)

    if not os.path.isfile(args.binary_file):
        print("Binary file not found")
        exit(1)

    filePatches = []
    with open(args.diff_file, "r") as diffFile:
        diffData = diffFile.read()
        diffLines = diffData.split("\n")

        for diffLine in diffLines:
            if ":" in diffLine:
                offset, values = diffLine.split(": ")
                original, patched = values.split(" ")
                filePatches.append({
                    "offset": int(offset, 16),
                    "original": original,
                    "patched": patched,
                })

    sections = []
    currentFileStart = 0
    pe = pefile.PE(args.binary_file)
    for section in pe.sections:
        sections.insert(0, {
            "name": section.Name.decode('utf-8'),
            "memoryOffset": section.VirtualAddress - section.PointerToRawData,
            "fileStart": currentFileStart
        })

        currentFileStart += section.SizeOfRawData

    initSectionWaring = False

    with open(f"{args.diff_file}_memory", "w") as memoryDiffFile:
        for index, filePatch in enumerate(filePatches):
            for section in sections:
                if section["fileStart"] < filePatch["offset"]:
                    if "INIT" in section["name"]:
                        if not initSectionWaring:
                            print("Diff file modifies init section, these patches are skipped")
                            initSectionWaring = True
                            
                        break

                    offset = section["memoryOffset"] + filePatch["offset"]
                    original = filePatch["original"]
                    patched = filePatch["patched"]

                    memoryDiffFile.write(f"{offset:02X} {original} {patched}\n")
                    break