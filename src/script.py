import commands

values = []
max_cores = 256
names = ["main_parallel", "cellular_ff", "cellular_ff_nopinning"]
points = [1,2,4,6,8,12,24,48,64,127,150,200,250,255]
ripetition = len(points)
for n in names:
    for i in points:
        media=0
        for j in range(ripetition):
            out =  commands.getstatusoutput('./'+n+' 500 500 1000 '+str(i))
            print(out[1])
            media += float(out[1].split()[4])
        print("Con par: "+str(i)+" terminato media: "+ str(media))
        values.append(media/ripetition)

    f = open("latency_"+n, "a")
    for ele in values:
        f.write(str(ele)+"\n")
    f.close()