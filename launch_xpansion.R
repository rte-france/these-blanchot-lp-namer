
#Tu charges le package :
library(antaresXpansion)

# Le chemin de l’étude :
setSimulationPath("D:\\boucle\\1- Base case", simulation = 0)
#setSimulationPath("\\local\\ssd\\manuruiz\\test_xpansion\\SmallTestSixCandidates_fast_1", simulation = 0)

# Le chemin du solver antares :
PathAntares <- "D:\\repo\\antares_v6.1rc1\\src\\solver\\RelWithDebInfo\\antares-7.0-solver.exe"

# On lance la boucle
benders(PathAntares)


#####