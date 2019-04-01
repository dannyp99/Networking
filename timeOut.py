#zero iteration
EstimatedRTT = 4.0
SampleRTT = 1.0
initDev = 1.0
delta = (1/8)
mu =1.0
phi = 4.0
Timeout = (mu*EstimatedRTT)+(phi*initDev)
#first iteration
difference = SampleRTT-EstimatedRTT
EstimatedRTT = EstimatedRTT +(delta*difference)
initDev= initDev + delta*(abs(difference)-initDev)
Timeout = (mu*EstimatedRTT)+(phi*initDev)

iterator = 2
while(Timeout>4.0):
    difference = SampleRTT-EstimatedRTT
    EstimatedRTT = EstimatedRTT +(delta*difference)
    initDev=initDev + delta*(abs(difference)-initDev)
    Timeout = (mu*EstimatedRTT)+(phi*initDev)
    print(str(Timeout)+"\n")
    iterator+=1

print("It took: " + str(iterator) + " iterations")

