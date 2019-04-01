#zero iteration
EstimatedRTT = 90
SampleRTT = 200
initDev = 50 
delta = (1/8)
limit = 300
mu =1
phi = 4
Timeout = (mu*EstimatedRTT)+(phi*initDev)

#first iteration
difference = SampleRTT-EstimatedRTT
EstimatedRTT = EstimatedRTT +(delta*difference)
initDev=initDev + delta*(abs(difference)-initDev)
Timeout = (mu*EstimatedRTT)+(phi*initDev)
print(str(Timeout)+"\n")

#second iteration
difference = SampleRTT-EstimatedRTT
EstimatedRTT = EstimatedRTT +(delta*difference)
initDev= initDev + delta*(abs(difference)-initDev)
Timeout = (mu*EstimatedRTT)+(phi*initDev)
print(str(Timeout)+"\n")


iterator = 2
while(Timeout>limit):
    difference = SampleRTT-EstimatedRTT
    EstimatedRTT = EstimatedRTT +(delta*difference)
    initDev=initDev + delta*(abs(difference)-initDev)
    Timeout = (mu*EstimatedRTT)+(phi*initDev)
    print(str(Timeout)+"\n")
    iterator+=1

print("It took: " + str(iterator) + " iterations for the Timeout to drop below "+str(limit))

