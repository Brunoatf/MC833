# 📦 Analisador de Pacotes ICMP (`package_analyzer.py`)

Este script realiza a análise de arquivos `.pcap` contendo pacotes ICMP, capturados por ferramentas como Wireshark e Mininet. Ele extrai métricas úteis sobre o tráfego de rede e gera gráficos que ajudam a visualizar o comportamento dos pacotes.

## 🔍 Funcionalidades

O script executa as seguintes tarefas:

- **Leitura dos arquivos**: `s1-eth1-200.pcap` e `s1-eth2-200.pcap`
Caso queria analisar outros pacotes é necess-ario alterar na função package_analyzer.py e adicnionar o nome dos novos arquivos em :

```bash
if __name__ == "__main__":
    arquivos_pcap = ["s1-eth1-200.pcap", "s1-eth2-200.pcap"]

    for arquivo in arquivos_pcap:
        analisar_pcap(arquivo)
```

- **Filtragem**: Considera apenas pacotes ICMP
- **Extração de métricas**:
  - Endereços IP de origem e destino
  - Contagem total de pacotes
  - Tamanho total transmitido (bytes)
  - Throughput médio (em bytes por segundo)
  - Intervalo médio entre pacotes (em segundos)
- **Geração de gráficos**:
  - Evolução do tamanho dos pacotes ao longo do tempo
  - Distribuição dos intervalos entre chegadas de pacotes (histograma)
  - Evolução do throughput por segundo

## 📦 Pré-requisitos

As dependências do projeto estão listadas no arquivo `requirements.txt`. Para instalá-las corretamente, use o `make`:

```bash
make install
```

Para rodar o script basta rodar o comando:

```bash
make all
```

Para limpar o venv e apagar as imagens criadas rode o comando:

```bash
make clean
```
