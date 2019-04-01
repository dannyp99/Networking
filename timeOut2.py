#zero iteration
EstimatedRTT = 90
SampleRTT = 200
initDev = 50
delta = (1/8)
mu =1
phi = 4
Timeout = (mu*EstimatedRTT)+(phi*initDev)

#init
difference = SampleRTT-EstimatedRTT
EstimatedRTT = EstimatedRTT +(delta*difference)
initDev=initDev + delta*(abs(difference)-initDev)
Timeout = (mu*EstimatedRTT)+(phi*initDev)
print(str(Timeout)+"\n")

#first iteration
difference = SampleRTT-EstimatedRTT
EstimatedRTT = EstimatedRTT +(delta*difference)
initDev= initDev + delta*(abs(difference)-initDev)
Timeout = (mu*EstimatedRTT)+(phi*initDev)
print(str(Timeout)+"\n")


iterator = 2
while(Timeout>300.0):
    difference = SampleRTT-EstimatedRTT
    EstimatedRTT = EstimatedRTT +(delta*difference)
    initDev=initDev + delta*(abs(difference)-initDev)
    Timeout = (mu*EstimatedRTT)+(phi*initDev)
    print(str(Timeout)+"\n")
    iterator+=1

print("It took: " + str(iterator) + " iterations")

