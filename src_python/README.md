# Pré-requis : dépôt en lien avec le code existant

<!-- A venir: Faire un build de la branche dev_xprs_win du dépôt https://devin-source.rte-france.com/antares/antares-historic.git -->

Faire un build de la branche master du dépôt https://devin-source.rte-france.com/antares/antares-xpansion-poc.git

Faire un build de la branche master du dépôt  https://devin-source.rte-france.com/rte-math-prog/stage-enzo.git


# Configurer le script de lancement

En attendant un déploiement stantardisé, il faut renseigner les repertoires et le nom des executables utilisé dans le script "launchXpansionV2.py"
 * lp_namer (lpnamerBinaryDir, lpname (rBinaryPath)
 * bendersmpi (mpi_binarypath, mpibenders)
 * mergemps (mergemps_binarypath, mergeMPS)

# Exemple d'exécution

 * Pour l'aide: python launchXpansionV2.py -h
 * Pour lancer seulement lpnamer: python launchXpansionV2.py -- step lp <Repertoire vers les données >
 * Pour l'optimisation benders mpi: python launchXpansionV2.py -- step optim <Repertoire vers les données > -solver bendersmpi -N 4
 * Pour l'optimisation mergemps : python launchXpansionV2.py -- step optim <Repertoire vers les données > -solver mergemps
 * Pour le calcul complet avec par exemple mergemps: python launchXpansionV2.py -- step optim <Repertoire vers les données> -solver mergemps



