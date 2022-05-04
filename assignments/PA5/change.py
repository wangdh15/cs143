import os

file_list = os.listdir("grading")
file_list = [x for x in file_list if x.endswith(".out")]
for file in file_list:
    with open(os.path.join("./grading", file), 'r') as f:
        cont = f.read()
    cont = cont.split("\n")
    cont[4] = "Loaded: ../lib/trap.handler"
    with open(os.path.join("./grading", file), 'w') as f:
        f.write("\n".join(cont))
