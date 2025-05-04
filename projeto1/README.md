
# MC833 - Projeto 1- Sistema de Streaming de Filmes (Cliente/Servidor TCP)

Este projeto simula um sistema de gerenciamento de filmes utilizando **sockets TCP em C**, com arquitetura **cliente-servidor**. O servidor mantém um **banco de dados em arquivo JSON**, e o cliente pode enviar requisições para **cadastrar, consultar, modificar ou remover filmes**.

---

## Estrutura do Projeto

- `client.c`: Aplicação cliente com interface via terminal.
- `server.c`: Aplicação servidor que responde a requisições e manipula o banco de dados.
- `movie.h`: Estrutura de dados dos filmes.

- `json_database.c/.h`: Funções auxiliares para leitura/escrita no arquivo `movies.json`.
- `cJSON.c/.h`: Biblioteca para manipulação de JSON em C.

- `movies.json`: Arquivo onde os filmes são armazenados (criado automaticamente).
- `Makefile`: Para compilar cliente e servidor.

---

## Como Compilar

No terminal, execute:

```bash
make
```

Isso vai gerar os executáveis:

- `./server`
- `./client`

---

## Como Executar

### 1. Inicie o servidor

```bash
./server
```

Você verá no terminal:

```
server: waiting for connections...
```

### 2. Inicie uma conexão

Em outro máquina:

```bash
./client <ip-máquina-servidor>
```
- Vale ressaltar que os testes foram feitos no mesmo WIFI utilizando computadores diferentes

Você verá uma **interface**:

```
Escolha uma opção:
1. Adicionar filme
2. Adicionar gênero a filme
3. Remover filme
4. Listar todos os títulos
5. Listar todos os filmes
6. Buscar filme por ID
7. Buscar filmes por gênero
0. Sair
```

---

## Exemplo de banco (`movies.json`)

```json
[
  {
    "id": 1,
    "title": "Inception",
    "genres": ["Sci-Fi", "Thriller"],
    "genre_count": 2,
    "director": "Christopher Nolan",
    "year": 2010
  }
]
```

---
