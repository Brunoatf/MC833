import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from scapy.all import *
import os

def carregar_pacotes_icmp(caminho_arquivo):
    pacotes = rdpcap(caminho_arquivo)
    return [pkt for pkt in pacotes if IP in pkt and ICMP in pkt]

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

def calcular_metricas(df):
    print("RASCUNHO")
    total_pacotes = len(df)
    print(f"Total de pacotes: {total_pacotes}")

    tempo_max = df["tempo"].max()
    tempo_min = df["tempo"].min()
    duracao = tempo_max - tempo_min
    print(f"Tempo inicial: {tempo_min}")
    print(f"Tempo final: {tempo_max}")
    print(f"Duração da captura (s): {duracao}")

    total_bytes = df["tamanho"].sum()
    print(f"Total de bytes transmitidos: {total_bytes}")

    throughput = total_bytes / duracao if duracao > 0 else 0
    print(f"Throughput (bytes/s): {throughput}")

    intervalo_medio = df["intervalo"].mean()
    print(f"Intervalo médio entre pacotes (s): {intervalo_medio}")

    return total_pacotes, throughput, intervalo_medio


def exibir_resultados(nome_arquivo, df, total_pacotes, throughput, intervalo_medio):
    print(f"--- Análise de {nome_arquivo} ---")
    print(f"Contagem de pacotes ICMP: {total_pacotes}")
    print(f"IPs de origem : {df['ip_origem'].unique()}")
    print(f"IPs de destino : {df['ip_destino'].unique()}")
    print(f"Throughput médio: {throughput:.2f} bytes/s")
    print(f"Intervalo médio entre pacotes: {intervalo_medio:.6f} s\n")
    df_enviados = df[df["ip_origem"] == "10.0.0.3"]  # ou o IP do h1
    print(len(df_enviados))


def gerar_graficos(df, nome_base):
    plot_tamanho_por_tempo(df, nome_base)
    plot_pacotes_por_tempo(df, nome_base)


def plot_tamanho_por_tempo(df, nome_base):
    plt.figure(figsize=(10, 6))
    plt.plot(df["tempo"], df["tamanho"], marker='o')
    plt.title(f"Tamanho dos Pacotes ao Longo do Tempo - {nome_base}")
    plt.xlabel("Tempo (s)")
    plt.ylabel("Tamanho do Pacote (bytes)")
    plt.grid(True)
    plt.savefig(f"{nome_base}_tamanho_vs_tempo.png")
    plt.close()

def plot_pacotes_por_tempo(df, nome_base):
    df["tempo"] = df["tempo"].astype(float)
    df["tempo_dt"] = pd.to_datetime(df["tempo"], unit='s')

    # Agrupa o número de pacotes por segundo
    df_contagem = df.set_index("tempo_dt").resample("1S").size()

    plt.figure(figsize=(12, 6))
    ax = df_contagem.plot(kind='bar', color='skyblue')

    plt.title(f"Número de Pacotes por Segundo - {nome_base}")
    plt.xlabel("Tempo")
    plt.ylabel("Número de Pacotes")
    plt.grid(True)

    # Personaliza os ticks: mostra só de 50 em 50 barras
    xticks = list(range(0, len(df_contagem), 50))
    xlabels = [df_contagem.index[i].strftime('%H:%M:%S') for i in xticks]
    ax.set_xticks(xticks)
    ax.set_xticklabels(xlabels, rotation=45)

    plt.tight_layout()
    plt.savefig(f"{nome_base}_pacotes_por_segundo.png")
    plt.close()

def analisar_pcap(caminho_arquivo):
    pacotes_icmp = carregar_pacotes_icmp(caminho_arquivo)
    df = extrair_dados(pacotes_icmp)
    total_pacotes, throughput, intervalo_medio = calcular_metricas(df)
    exibir_resultados(caminho_arquivo, df, total_pacotes, throughput, intervalo_medio)
    gerar_graficos(df, caminho_arquivo)

if __name__ == "__main__":
    #arquivos_pcap = ["s1-eth1-200.pcap", "s1-eth3-200.pcap", "s1-eth2-200.pcap", "s1-eth4-200.pcap"]
    arquivos_pcap = ["s1-eth1-200.pcap"]

    for arquivo in arquivos_pcap:
        analisar_pcap(arquivo)
