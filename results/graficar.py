#!/usr/bin/env python3
"""Genera las graficas de la Seccion 8 a partir de results/results_fb.csv y
results/results_bt.csv. Se deja en results/ junto con sus salidas para que el
docente pueda reproducir las figuras (Seccion 11: "graficas (.png)").

Uso (desde la raiz del repositorio):
    python3 results/graficar.py
"""
import csv
import os
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

# Paleta categorica fija (no ciclica): FB=azul, Diccionario=naranja,
# con poda=verde azulado, sin poda=gris. Se usa consistentemente en
# todas las figuras para que el color siempre identifique al mismo metodo.
COLOR_FB = "#3B7DD8"
COLOR_DICC = "#E07B39"
COLOR_CON_PODA = "#2E8B7A"
COLOR_SIN_PODA = "#8C8C8C"


def leer_csv(ruta):
    with open(ruta, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def graficar_fb(filas, out_path):
    # Tiempo (ms) vs longitud de la contraseña objetivo, una serie por
    # metodo (FB vs Diccionario). La longitud es un proxy directo del
    # "tamaño de entrada" para el Modulo FB (Seccion 8).
    longitudes = {
        "Referencia": 5,
        "Equipo-1": 4, "Equipo-2": 4, "Equipo-3": 5, "Equipo-4": 5, "Equipo-5": 6,
    }

    fig, ax = plt.subplots(figsize=(7.5, 5), dpi=150)
    for metodo, color, marker in [("FB", COLOR_FB, "o"), ("Dicc", COLOR_DICC, "s")]:
        xs, ys, labels = [], [], []
        for fila in filas:
            if fila["metodo"] != metodo:
                continue
            inst = fila["instancia"]
            n = longitudes.get(inst)
            if n is None:
                continue
            xs.append(n)
            ys.append(float(fila["tiempo_ms"]))
            labels.append(inst)
        ax.scatter(xs, ys, color=color, marker=marker, s=60, label=("Fuerza bruta" if metodo == "FB" else "Diccionario"), zorder=3)
        for x, y, lab in zip(xs, ys, labels):
            ax.annotate(lab, (x, y), textcoords="offset points", xytext=(6, 4), fontsize=7, color="#444444")

    ax.set_yscale("log")
    ax.set_xlabel("Longitud de la contraseña objetivo (caracteres)")
    ax.set_ylabel("Tiempo de ejecución (ms, escala log)")
    ax.set_title("Módulo FB — Tiempo de ejecución vs. longitud de la contraseña\n(fuerza bruta pura vs. ataque por diccionario)")
    ax.grid(True, which="both", axis="y", linestyle="--", linewidth=0.5, color="#DDDDDD", zorder=0)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.legend(frameon=False)
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)
    print(f"[ok] {out_path}")


def graficar_fb_candidatos(filas, out_path):
    longitudes = {
        "Referencia": 5,
        "Equipo-1": 4, "Equipo-2": 4, "Equipo-3": 5, "Equipo-4": 5, "Equipo-5": 6,
    }
    fig, ax = plt.subplots(figsize=(7.5, 5), dpi=150)
    for metodo, color, marker in [("FB", COLOR_FB, "o"), ("Dicc", COLOR_DICC, "s")]:
        xs, ys = [], []
        for fila in filas:
            if fila["metodo"] != metodo:
                continue
            inst = fila["instancia"]
            n = longitudes.get(inst)
            if n is None:
                continue
            xs.append(n)
            ys.append(int(fila["candidatos_evaluados"]))
        ax.scatter(xs, ys, color=color, marker=marker, s=60, label=("Fuerza bruta" if metodo == "FB" else "Diccionario"), zorder=3)

    ax.set_yscale("log")
    ax.set_xlabel("Longitud de la contraseña objetivo (caracteres)")
    ax.set_ylabel("Candidatos evaluados (escala log)")
    ax.set_title("Módulo FB — Candidatos evaluados vs. longitud\n(crecimiento empírico del costo)")
    ax.grid(True, which="both", axis="y", linestyle="--", linewidth=0.5, color="#DDDDDD", zorder=0)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.legend(frameon=False)
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)
    print(f"[ok] {out_path}")


def graficar_bt_nodos(filas, out_path):
    # Nodos visitados (con poda) y nodos generados (sin poda, cuando se
    # ejecuto) vs. n, para el barrido con la politica completa del equipo
    # (Seccion 8: "grafica tiempo de ejecucion vs. tamaño de entrada" --
    # aqui se usa nodos en vez de tiempo porque casi todas las instancias
    # con n>=5 llegan al limite de nodos en un tiempo casi constante
    # (~2s); los nodos visitados SI reflejan el crecimiento real del
    # espacio, que es la cantidad que exige cuantificar la Seccion 6.2).
    barrido = [f for f in filas if f["instancia"].startswith("Barrido n=")]
    barrido.sort(key=lambda f: int(f["n"]))

    ns = [int(f["n"]) for f in barrido]
    con_poda = [int(f["nodos_visitados_con_poda"]) for f in barrido]
    sin_poda = [int(f["nodos_sin_poda"]) if f["nodos_sin_poda"] not in ("", "-1") else None for f in barrido]
    estados = [f["estado"] for f in barrido]

    fig, ax = plt.subplots(figsize=(7.5, 5), dpi=150)
    ax.plot(ns, con_poda, color=COLOR_CON_PODA, marker="o", linewidth=2, label="Con poda (backtracking)", zorder=3)
    ns_sp = [n for n, v in zip(ns, sin_poda) if v is not None]
    vs_sp = [v for v in sin_poda if v is not None]
    if ns_sp:
        ax.plot(ns_sp, vs_sp, color=COLOR_SIN_PODA, marker="s", linewidth=2, linestyle="--", label="Sin poda (enumeración exhaustiva)", zorder=3)

    for n, v, estado in zip(ns, con_poda, estados):
        marca = " (parcial: llegó al límite de nodos)" if estado == "parcial" else " (completo)"
        ax.annotate(marca.strip(), (n, v), textcoords="offset points", xytext=(0, 8), fontsize=6, color="#444444", ha="center")

    ax.set_yscale("log")
    ax.set_xlabel("n (longitud de la contraseña)")
    ax.set_ylabel("Nodos (escala log)")
    ax.set_title("Módulo BT — Nodos explorados vs. n (política completa del equipo)\ncon poda vs. sin poda (barrido suplementario)")
    ax.set_xticks(ns)
    ax.grid(True, which="both", axis="y", linestyle="--", linewidth=0.5, color="#DDDDDD", zorder=0)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.legend(frameon=False)
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)
    print(f"[ok] {out_path}")


def graficar_bt_reduccion(filas, out_path):
    # Porcentaje de reduccion del espacio de busqueda (con poda vs. sin
    # poda) para las instancias donde SI se pudo comparar contra la
    # enumeracion exhaustiva completa (Seccion 8.2).
    candidatas = []
    for f in filas:
        try:
            con_poda = int(f["nodos_visitados_con_poda"])
            sin_poda = int(f["nodos_sin_poda"])
        except ValueError:
            continue
        if sin_poda <= 0 or f["estado"] != "completo":
            continue
        reduccion = 100.0 * (1.0 - con_poda / sin_poda)
        candidatas.append((f["instancia"], reduccion))

    if not candidatas:
        print("[info] no hay instancias 'completo' con comparacion sin-poda disponible; se omite graficar_bt_reduccion")
        return

    nombres = [c[0] for c in candidatas]
    valores = [c[1] for c in candidatas]

    fig, ax = plt.subplots(figsize=(7.5, 4.5), dpi=150)
    ax.barh(nombres, valores, color=COLOR_CON_PODA, zorder=3)
    for i, v in enumerate(valores):
        ax.annotate(f"{v:.1f}%", (v, i), textcoords="offset points", xytext=(4, 0), va="center", fontsize=8)
    ax.set_xlabel("Reducción del espacio de búsqueda (%)")
    ax.set_title("Módulo BT — Reducción del espacio por poda\n(instancias con comparación exhaustiva completa)")
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.set_xlim(0, 105)
    fig.tight_layout()
    fig.savefig(out_path)
    plt.close(fig)
    print(f"[ok] {out_path}")


def main():
    fb_csv = os.path.join(HERE, "results_fb.csv")
    bt_csv = os.path.join(HERE, "results_bt.csv")

    if os.path.exists(fb_csv) and os.path.getsize(fb_csv) > 0:
        filas_fb = leer_csv(fb_csv)
        graficar_fb(filas_fb, os.path.join(HERE, "grafica_fb_tiempo.png"))
        graficar_fb_candidatos(filas_fb, os.path.join(HERE, "grafica_fb_candidatos.png"))
    else:
        print(f"[skip] {fb_csv} no existe o esta vacio -- corre bin/ada_p1_fb primero")

    if os.path.exists(bt_csv) and os.path.getsize(bt_csv) > 0:
        filas_bt = leer_csv(bt_csv)
        graficar_bt_nodos(filas_bt, os.path.join(HERE, "grafica_bt_nodos.png"))
        graficar_bt_reduccion(filas_bt, os.path.join(HERE, "grafica_bt_reduccion.png"))
    else:
        print(f"[skip] {bt_csv} no existe o esta vacio -- corre bin/ada_p1_bt primero")


if __name__ == "__main__":
    main()
