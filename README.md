[![C99](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf)

# Mini-Servidor Web
 Implementação parcial de um servidor _Web_ utilizando 4 técnicas distintas de programação com _sockets_. O servidor lida com mensagens _HTTP_ (_parser_) apenas com o método _GET_, utilizando o protocolo _TCP_ na camada de transporte (Trabalho Prático 2 da disciplina de Redes de Computadores I - DCOMP - UFSJ). 
 
# Requisitos

- Ferramenta [Siege](https://www.joedog.org/siege-home/)

       sudo apt install siege -y
 
# Compilação
     
       make
       
# Execução

- Servidor Iterativo:

      make iterative
      
- Servidor Fork:

      make fork
      
- Servidor Thread:

      make thread
      
- Servidor Concorrente:

      make concurrent
      
 # Benchmark Siege
 
- Formato de execução geral:
 
   `siege -t<tempo em segundos>S -c<número de clientes simultâneos> http://localhost:<número da porta>/<nome do arquivo>`
 
- Exemplo:
 
      siege -t10S -c128 http://localhost:2000/cat.jpg
       
 # Browser
 
- Formato de pesquisa geral:
 
   `http://127.0.0.1:<número da porta>/<nome do arquivo>`
 
- Exemplo:
 
      http://127.0.0.1:2000/index.html
  
  
      
      
      
 
 
