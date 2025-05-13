import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from scapy.all import *
import os

# Carrega pacotes de um arquivo .pcap e filtra apenas os pacotes ICMP com camada IP
def carregar_pacotes_icmp(caminho_arquivo):
    pacotes = rdpcap(caminho_arquivo)
    return [pkt for pkt in pacotes if IP in pkt and ICMP in pkt]

# Extrai dados relevantes dos pacotes ICMP: tempo, IPs de origem e destino, tamanho
def extrair_dados(pacotes_icmp):
    dados = {
        "tempo": [pkt.time for pkt in pacotes_icmp],
        "ip_origem": [pkt[IP].src for pkt in pacotes_icmp],
        "ip_destino": [pkt[IP].dst for pkt in pacotes_icmp],
        "tamanho": [len(pkt) for pkt in pacotes_icmp]
    }
    df = pd.DataFrame(dados)
    df["intervalo"] = df["tempo"].diff() 
    return df

# Calcula métricas estatísticas da captura de pacotes ICMP:
# - total de pacotes capturados,
# - duração da captura ,
# - total de bytes transmitidos,
# - throughput médio (em bytes por segundo),
# - intervalo médio entre pacotes consecutivos.
def calcular_metricas(df):
    total_pacotes = len(df) 

    tempo_max = df["tempo"].max()  
    tempo_min = df["tempo"].min()  

    duracao = tempo_max - tempo_min  

    total_bytes = df["tamanho"].sum()  

    throughput = total_bytes / duracao if duracao > 0 else 0 
    intervalo_medio = df["intervalo"].mean()  

    return total_pacotes, throughput, intervalo_medio

# Exibe os principais resultados calculados
def exibir_resultados(nome_arquivo, df, total_pacotes, throughput, intervalo_medio):
    print(f"--- Análise de {nome_arquivo} ---")
    print(f"Contagem de pacotes ICMP: {total_pacotes}")
    print(f"IPs de origem : {df['ip_origem'].unique()}")
    print(f"IPs de destino : {df['ip_destino'].unique()}")
    print(f"Throughput médio: {throughput:.2f} bytes/s")
    print(f"Intervalo médio entre pacotes: {intervalo_medio:.6f} s\n")
    df_enviados = df[df["ip_origem"] == "10.0.0.3"]  
    print(len(df_enviados))

# Gera quatro gráficos: t
# - Tamanho dos pacotes por tempo 
# - Quantidade de pacotes por tempo
# - Distribuição de intervalos entre os pacotes
# - Throughput em função do tempo
def gerar_graficos(df, nome_base):
    plot_tamanho_por_tempo(df, nome_base)
    plot_pacotes_por_tempo(df, nome_base)
    plot_throughput_por_tempo(df, nome_base)
    plot_distribuicao_intervalos(df, nome_base)

# Gera gráfico do tamanho dos pacotes ICMP ao longo do tempo
def plot_tamanho_por_tempo(df, nome_base):
    plt.figure(figsize=(10, 6))
    plt.plot(df["tempo"], df["tamanho"], marker='o')
    plt.title(f"Tamanho dos Pacotes ao Longo do Tempo - {nome_base}")
    plt.xlabel("Tempo (s)")
    plt.ylabel("Tamanho do Pacote (bytes)")
    plt.grid(True)
    plt.savefig(f"{nome_base}_tamanho_vs_tempo.png")
    plt.close()

# Gera gráfico de número de pacotes ICMP por segundo
def plot_pacotes_por_tempo(df, nome_base):
    df["tempo"] = df["tempo"].astype(float)
    df["tempo_dt"] = pd.to_datetime(df["tempo"], unit='s')

    df_contagem = df.set_index("tempo_dt").resample("1S").size()

    plt.figure(figsize=(12, 6))
    ax = df_contagem.plot(kind='bar', color='skyblue')

    plt.title(f"Número de Pacotes por Segundo - {nome_base}")
    plt.xlabel("Tempo")
    plt.ylabel("Número de Pacotes")
    plt.grid(True)

    xticks = list(range(0, len(df_contagem), 50))
    xlabels = [df_contagem.index[i].strftime('%H:%M:%S') for i in xticks]
    ax.set_xticks(xticks)
    ax.set_xticklabels(xlabels, rotation=45)

    plt.tight_layout()
    plt.savefig(f"{nome_base}_pacotes_por_segundo.png")
    plt.close()

# Calcula o throughput (quantidade de dados transmitidos por segundo, em bytes/s) ao longo do tempo
def plot_throughput_por_tempo(df, nome_base):
    df["tempo_dt"] = pd.to_datetime(df["tempo"], unit='s')
    df = df.set_index("tempo_dt")

    throughput_por_segundo = df["tamanho"].resample("1S").sum()

    plt.figure(figsize=(12, 6))
    plt.plot(throughput_por_segundo.index, throughput_por_segundo.values, color='skyblue', marker='o')
    plt.title(f"Throughput por Segundo - {nome_base}")
    plt.xlabel("Tempo")
    plt.ylabel("Throughput (bytes/s)")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"{nome_base}_throughput_por_tempo.png")
    plt.close()

# Calcula a distribuição dos intervalos de tempo entre pacotes consecutivos
def plot_distribuicao_intervalos(df, nome_base):
    plt.figure(figsize=(10, 6))
    plt.hist(df["intervalo"].dropna(), bins=50, color='skyblue', edgecolor='black')
    plt.title(f"Distribuição dos Intervalos entre Pacotes - {nome_base}")
    plt.xlabel("Intervalo entre pacotes (s)")
    plt.ylabel("Frequência")
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"{nome_base}_distribuicao_intervalos.png")
    plt.close()



# Função principal que coordena a análise de um arquivo .pcap
def analisar_pcap(caminho_arquivo):
    pacotes_icmp = carregar_pacotes_icmp(caminho_arquivo)
    df = extrair_dados(pacotes_icmp)
    total_pacotes, throughput, intervalo_medio = calcular_metricas(df)
    exibir_resultados(caminho_arquivo, df, total_pacotes, throughput, intervalo_medio)
    gerar_graficos(df, caminho_arquivo)

# Funcao de execucao dos arquivos .pcap
if __name__ == "__main__":
    arquivos_pcap = ["s1-eth1-200.pcap", "s1-eth2-200.pcap"]

    for arquivo in arquivos_pcap:
        analisar_pcap(arquivo)
