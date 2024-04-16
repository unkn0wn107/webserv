# webserv
Simple http/1.1 (bis) server implemented in c++ for 42 school cursus

## RFC : [Résumé des différents documents de la RFC](doc/RFC.md)

## [Diagramme de classes](doc/UML.mmd)

## clang-format / clang-tidy
1. Install llvm
 - Debian / Ubuntu : `sudo apt update && sudo apt install clang-format clang-tidy`
 - Fedora / centOS : `sudo dnf upgrade && sudo dnf install llvm`
2. (Optional) Usage - automatic
 - [Install Trunk.io vscode plugin](https://marketplace.visualstudio.com/items?itemName=trunk.io) : `ext install Trunk.io`
 - Install linter deps : `trunk install`
 - Check (already auto in .vscode/settings) : `trunk check`
3. Usage - manual
 - Format all files : `find . -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.tpp" | xargs clang-format -i`
 - Static analysis : `find . -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.tpp" | xargs clang-tidy -config=.clang-tidy`
