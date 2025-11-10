# korneevscp-INJ

Petit utilitaire Win32 en C pour démonstration / usage en laboratoire.
Lisez la section « Sécurité & Légalité » avant toute utilisation.

# Description

Application graphique Windows (API Win32) qui permet de rechercher un processus par nom et d’injecter une DLL dans ce processus via allocation mémoire distante et CreateRemoteThread(LoadLibraryA). Interface minimale : champ pour le nom du processus, champ pour le chemin de la DLL, bouton Browse et bouton Inject.

But pédagogique : illustrer l’utilisation des API Windows (Toolhelp32, OpenProcess, VirtualAllocEx, WriteProcessMemory, CreateRemoteThread) et la création d’une fenêtre Win32 simple.

# Avertissement — Sécurité & Légalité (lire impérativement)

Ce logiciel peut être employé à des fins malveillantes (injection de code).

N’utilisez pas ce programme pour compromettre, contourner des protections, ou agir sur des systèmes qui ne vous appartiennent pas.

Testez uniquement sur des machines et des processus dont vous avez la pleine autorisation (votre PC, une VM de test, un environnement de labo).

Je ne fournirai pas d’assistance pour une utilisation illégale ou non autorisée.

# Fonctionnalités

Interface GUI Win32 (EDIT, BUTTON, STATIC).

Recherche du PID d’un processus par son nom (Toolhelp32).

Injection de DLL via VirtualAllocEx, WriteProcessMemory et CreateRemoteThread.

Boîte de dialogue standard Windows pour sélectionner la DLL.

Apparence texte cyan sur fond noir (style simple).

# Prérequis

OS : Windows.

Compilateur : Code::Blocks (MinGW), MinGW-w64, ou MSVC.

Bibliothèque : comctl32 (le code inclut #pragma comment(lib, "comctl32.lib")).

Droits : l’accès à certains processus peut nécessiter des privilèges administrateur.

Compilation
Avec Code::Blocks (MinGW) — méthode recommandée si vous avez déjà utilisé Code::Blocks

Ouvrez Code::Blocks → File → New → Project → Empty project ou Console/GUI project.

Ajoutez le fichier source .c au projet.

Dans Project → Build options → Linker settings, ajoutez comctl32.

(Optionnel) Pour une application sans console, ajoutez -mwindows aux options du linker (Project → Build options → Other linker options).

Build → Build.

Avec MinGW-w64 / GCC (ligne de commande)
gcc -o korneevscp-INJ.exe votre_fichier.c -lcomctl32 -mwindows


-mwindows rend l’exécutable GUI (pas de console). Retirez-le si vous voulez une console attachée.

Avec MSVC (Visual Studio)

Créez un projet Win32 (Windows Desktop Application) et ajoutez le fichier source.

Ajoutez comctl32.lib aux dépendances du linker si nécessaire.

Build.

# Utilisation

Lancez l’exécutable (préférablement dans une VM de test).

Dans Process name : entrez le nom exact du binaire cible (ex. notepad.exe).

Dans DLL path : utilisez Browse pour sélectionner une DLL (idéalement une DLL de test écrite par vous).

Cliquez sur Inject.

Consultez le champ Status pour le résultat : Process not found., Injection success. ou Injection failed.

Exemple de DLL de test (sécurisé)

Pour tester sans risque, créez une petite DLL qui affiche simplement une boîte de dialogue lors du chargement (DllMain + MessageBoxA). Testez ceci dans une VM.

Limitations & améliorations possibles

Le code utilise PROCESS_ALL_ACCESS : l’outil échouera souvent si l’utilisateur n’a pas les droits nécessaires.

Validation minimale des entrées (trim, vérification de l’extension .exe, etc.).

Logging/verbose pour faciliter le debug.

Remplacement de CreateRemoteThread par des méthodes plus robustes si ciblé pour usage avancé (attention aux implications légales et éthiques).

# Dépannage

Process not found. — Vérifiez que le processus est bien en cours d’exécution et que le nom est exact (incluant .exe si nécessaire).

Injection failed. — Vérifiez les droits (exécutez en administrateur dans la VM), le chemin de la DLL, et que la DLL est compatible (32-bit vs 64-bit).

Problèmes de compilation — Assurez-vous d’avoir les headers Windows (<windows.h>, <tlhelp32.h>) et que vous compilez sous Windows.

# Tests recommandés (responsables)

Créez une VM Windows dédiée.

Utilisez un exécutable de test que vous contrôlez et une DLL de test inoffensive (ex. affiche un message).

Ne testez pas sur machines de production ni sur des cibles qui ne vous appartiennent pas.



# Crédits

Auteur : korneevscp (compilé avec Code::Blocks).

