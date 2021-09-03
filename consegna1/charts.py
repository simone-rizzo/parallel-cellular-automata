import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import interp1d
from scipy.interpolate import interpolate
from scipy.signal import savgol_filter

lista_mine_write = []
lista_ff_parfor_write = []
lista_main = []
# test = "400x400 iter 20"
# t_seq = 12542900
# t_seq = 9381035
#t_seq = 8829397 # 500 500 1000
t_seq = 115936403 # 3000 3000 100
#t_seq = 3409536
x = np.array([i for i in range(1, 257)])
with open("./time/latency_mine_2") as f:
    content_mine_write = f.readlines()
with open("./time/latency_ff_parfor_2") as f:
    content_ff_parfor_write = f.readlines()
with open("./time/latency_ff_farm_2") as f:
    content = f.readlines()

for ele in content_mine_write:
    lista_mine_write.append(float(ele.split("\n")[0]))
for ele in content_ff_parfor_write:
    lista_ff_parfor_write.append(float(ele.split("\n")[0]))
for ele in content:
    lista_main.append(float(ele.split("\n")[0]))

lista_mine_write1 = np.array([(t_seq/ele)/(i+1) for i, ele in enumerate(lista_mine_write)]) #Efficency
lista_ff_parfor_write1 = np.array([(t_seq/ele)/(i+1) for i, ele in enumerate(lista_ff_parfor_write)]) #Efficency
lista_main1 = np.array([(t_seq/ele)/(i+1) for i, ele in enumerate(lista_main)]) #Efficency

lista_mine_write2 = np.array([(lista_mine_write[0] / ele) for i, ele in enumerate(lista_mine_write)])  # Scalability
lista_ff_parfor_write2 = np.array([(lista_ff_parfor_write[0] / ele) for i, ele in enumerate(lista_ff_parfor_write)])  # Scalability
lista_main2 = np.array([(lista_main[0] / ele) for i, ele in enumerate(lista_main)])  # Scalability

lista_mine_write3 = np.array([(t_seq / ele) for i, ele in enumerate(lista_mine_write)])  # Speedup
lista_ff_parfor_write3 = np.array([(t_seq / ele) for i, ele in enumerate(lista_ff_parfor_write)])  # Speedup
lista_main3 = np.array([(t_seq / ele) for i, ele in enumerate(lista_main)])  # Speedup

lista_mine_write4 = np.array([ele/1000000 for i, ele in enumerate(lista_mine_write)])  # Latency
lista_ff_parfor_write4 = np.array([ele/1000000 for i, ele in enumerate(lista_ff_parfor_write)])  # Latency
lista_main4 = np.array([ele/1000000 for i, ele in enumerate(lista_main)])  # Latency


"""Latency"""
plt.figure(figsize=(11, 6))

# f = savgol_filter(lista_mine_write4, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(x, lista_mine_write4, ":",color='black', label="mine")
min = lista_mine_write4.min()
pos = np.argwhere(lista_mine_write4 == min)[0]+1
print(str(min)+" "+str(pos)+" no pinned")
plt.plot(pos, min, 'go', color='black')

# plt.plot(x, f, '--',  color='black')
plt.plot(x, lista_ff_parfor_write4, "--", label="ff_parfor",color='black')
min = lista_ff_parfor_write4.min()
pos = np.argwhere(lista_ff_parfor_write4 == min)[0]+1
print(str(min)+" "+str(pos)+" ff")
plt.plot(pos, min, 'go', color='black')

# plt.plot(x, f, '--',  color='black')
plt.plot(np.array([i for i in range(2, 257)]), lista_main4, label="ff_farm",color='black')
min = lista_main4.min()

pos = np.argwhere(lista_main4 == min)[0]+1
print(str(min)+" "+str(pos)+" main")
plt.plot(pos, min, 'go', color='black')
plt.xlabel("parDegree")
plt.ylabel("ElapsedTime(sec)")
plt.legend()
plt.title("ElapsedTime \n ", pad=20)
plt.grid()
plt.show()

# Speed up
plt.figure(figsize=(11, 6))
# x = np.array([i for i in range(1, 256)])
#f = savgol_filter(lista_mine_write3, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(x, lista_mine_write3, ":", color='black', label="mine")
min = lista_mine_write3.max()
pos = np.argwhere(lista_mine_write3 == min)[0]+1
print(str(min)+" "+str(pos)+" no pinned")
plt.plot(pos, min, 'go', color='black')

#f = savgol_filter(lista_ff_parfor_write3, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(x, lista_ff_parfor_write3, "--", color='black', label="ff_parfor")
min = lista_ff_parfor_write3.max()
pos = np.argwhere(lista_ff_parfor_write3 == min)[0]+1
print(str(min)+" "+str(pos)+" no pinned")
plt.plot(pos, min, 'go', color='black')

#f = savgol_filter(lista_main3, 51, 3)
# plt.plot(x, f, '--',  color='black')
#2743299.8
plt.plot(np.array([i for i in range(2, 257)]), lista_main3, "-", color='black', label="ff_farm")
min = lista_main3.max()
pos = np.argwhere(lista_main3 == min)[0]+1
print(str(min)+" "+str(pos)+" no pinned")
plt.plot(pos, min, 'go', color='black')
plt.xlabel("parDegree")
plt.ylabel("speedup")
plt.legend()
plt.title("SpeedUp \n ", pad=20)
plt.grid()
plt.show()

#Scalability
plt.figure(figsize=(11, 6))
# x = np.array([i for i in range(1, 256)])
#f = savgol_filter(lista_mine_write2, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(x, lista_mine_write2, ":", color='black', label="mine")
min = lista_mine_write2.max()
pos = np.argwhere(lista_mine_write2 == min)[0]+1
print(str(min)+" "+str(pos)+" no pinned")
plt.plot(pos, min, 'go',  color="black")

#f = savgol_filter(lista_ff_parfor_write2, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(x, lista_ff_parfor_write2, "--", color='black', label="ff_parfor")
min = lista_ff_parfor_write2.max()
pos = np.argwhere(lista_ff_parfor_write2 == min)[0]+1

print(str(min)+" "+str(pos)+" no pinned")
plt.plot(pos, min, 'go' , color="black")

#f = savgol_filter(lista_main2, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(np.array([i for i in range(2, 257)]), lista_main2, color='black', label="ff_farm")
min = lista_main2.max()
pos = np.argwhere(lista_main2 == min)[0]+1
print(str(min)+" "+str(pos)+" no pinned")
plt.plot(pos, min, 'go', color="black")
plt.xlabel("parDegree")
plt.ylabel("scalability")
plt.legend()
plt.title("Scalability \n ", pad=20)
plt.grid()
plt.show()

#Efficiency
plt.figure(figsize=(11, 6))
# x = np.array([i for i in range(1, 256)])
#f = savgol_filter(lista_mine_write1, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(x, lista_mine_write1,":",color='black', label="mine")
g = np.array([0.600 for i in x])
"""idx = np.argwhere(np.diff(np.sign(lista_mine_write1 - g))).flatten()
plt.plot(x[idx], lista_mine_write1[idx], 'ro')"""

#f = savgol_filter(lista_ff_parfor_write1, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(x, lista_ff_parfor_write1,"--", color='black', label="ff_parfor")
g = np.array([0.600 for i in x])
"""idx = np.argwhere(np.diff(np.sign(lista_ff_parfor_write1 - g))).flatten()
plt.plot(x[idx], lista_ff_parfor_write1[idx], 'ro')"""

#f = savgol_filter(lista_main1, 51, 3)
# plt.plot(x, f, '--',  color='black')
plt.plot(np.array([i for i in range(2, 257)]), lista_main1, color='black', label="ff_farm")
g = np.array([0.600 for i in x])
"""idx = np.argwhere(np.diff(np.sign(lista_main1 - g))).flatten()
plt.plot(x[idx], lista_main1[idx], 'ro')"""
plt.plot(x, g,color='black')
plt.xlabel("parDegree")
plt.ylabel("efficiency")
plt.legend()
plt.title("Efficiency \n ", pad=20)
plt.grid()
plt.yticks(np.arange(0, 1.1, 0.1))
plt.ylim(0, 1)
plt.show()

# Theoretic time vs actual time
plt.figure(figsize=(11, 6))

# x = np.array([i for i in range(1, 256)])
y_ideal = [t_seq/(ele*1000000) for ele in x]

plt.plot(x, lista_ff_parfor_write4, label="Actual time", color="black")

min = lista_ff_parfor_write4.min()
pos = np.argwhere(lista_ff_parfor_write4 == min)[0]
# plt.plot(pos, min, 'go')

plt.plot(x, y_ideal,"--", label='Theoretical time', color='black')
plt.xlabel("parDegree")
plt.ylabel("ElapsedTime(sec)")
plt.legend()
plt.title("Theoretical time vs Actual time", pad=20)
plt.grid()
plt.show()


"""with open("image_creation") as f:
    content = f.readlines()
somma = 0
for ele in content:
    somma += float(ele.split()[3])
print(somma/len(content))"""


