## Repositório para organização de trabalhos de arquivos 
Shogo e Pietra 

Para rodar os programas em C, do diretório desejado (e.g. "T1"), para testar casos (e.g. "1.in"):
```
cd T1
make all
make run < test/1.in
```
Caso queira debugar com Valgrind:
```
make debug < test/1.in
```
Para apagar todos os arquivos objeto:
```
make clean
```
