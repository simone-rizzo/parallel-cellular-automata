import commands
import numpy as np

values = []
ripetition = 5
max_cores = 256
for i in range(1,max_cores):
    media=0
    for j in range(ripetition):
        out =  commands.getstatusoutput('./main 800 800 10 '+str(i))
        print(out[1])
        media += float(out[1].split()[4])
    print("Con par: "+str(i)+" terminato media: "+ str(media))
    values.append(media/ripetition)

f = open("latency_result", "a")
for ele in values:
    f.write(str(ele)+"\n")
f.close()


"""If there is mathplotlib"""
"""import matplotlib.pyplot as plt
lista = []
with open("latency_result") as f:
    content = f.readlines()
    print(content[2])
    for ele in content:         
        lista.append(float(ele.split("\n")[0])/float(5))
    plt.plot([i for i in range(1,256)], lista)
    plt.show()"""

