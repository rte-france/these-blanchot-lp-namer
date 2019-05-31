# Illustration de la proposition d'industrialisation de AntaresXpansion



## 1. Pré-requis

Faire un build de la branche dev_xprs_win du dépôt https://devin-source.rte-france.com/antares/antares-historic.git

Faire un build de la branche master du dépôt https://devin-source.rte-france.com/antares/antares-xpansion-poc.git



## 2. Déroulement

Lancer la simulation ANTARES avec les paramètres suivants:

*include-exportmps = true*
*include-exportstructure = true*

aller dans le dossier output qui contient les sorties de la simulation ANTARES standard ainsi que de nouvelles informations.

exéctuer example.bat:

​	mkdir lp

​	copy area* area.txt

​	copy interco* interco.txt

​	python get_names_py > mps.txt

​	..\\..\\..\build\\RelWithDebInfo\lp_namer.exe .

Lancer la résolution

benderssequential . : benders en séquentiel

mpiexec -n 4 bendersmpi . : benders en parallèle

merge_mps . : fusion des problèmes et résolution en frontal

