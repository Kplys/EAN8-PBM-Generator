import tkinter as tk, tkinter.filedialog as fd, tkinter.simpledialog as sd, tkinter.messagebox as mb
import subprocess, os, sys

def gerar():
    id7 = sd.askstring("ID", "Identificador (7 dígitos):")
    if not id7:
        return
    id7 = id7.strip().replace(" ", "")   # <<< ESSENCIAL
    if len(id7) != 7 or not id7.isdigit():
        mb.showerror("Erro", "ID deve ter exatamente 7 dígitos numéricos.")
        return

    out = fd.asksaveasfilename(defaultextension=".pbm", filetypes=[("PBM","*.pbm")], initialfile="barcode.pbm")
    if not out:
        return

    try:
        px = sd.askinteger("px/area", "Pixels por area (default 3):", initialvalue=3, minvalue=1)
        sp = sd.askinteger("spacing", "Espaçamento lateral (default 4):", initialvalue=4, minvalue=0)
        h = sd.askinteger("height", "Altura do código (default 50):", initialvalue=50, minvalue=10)
    except:
        mb.showerror("Erro", "Parâmetros inválidos")
        return

    binario = mb.askyesno("Binario?", "Salvar em binario (P4)?")

    cmd = ["gerador.exe", "-i", id7, "-o", out, "-w", str(px), "-s", str(sp), "-h", str(h)]
    if binario:
        cmd.append("-b")

    try:
        subprocess.run(cmd, check=True)
        mb.showinfo("OK", f"Arquivo gerado: {out}")
    except Exception as e:
        mb.showerror("Erro", str(e))


def extrair():
    f = fd.askopenfilename(filetypes=[("PBM","*.pbm")])
    if not f: return
    cmd = ["extrator.exe" if os.name == "nt" else "./extrator", f]
    try:
        proc = subprocess.run(cmd, capture_output=True, text=True)
        if proc.returncode == 0:
            mb.showinfo("ID extraído", proc.stdout.strip())
        else:
            mb.showerror("Erro", proc.stdout.strip() or proc.stderr.strip())
    except Exception as e:
        mb.showerror("Erro", str(e))

root = tk.Tk()
root.title("EAN-8 PBM GUI")
tk.Button(root, text="Gerar código", width=30, command=gerar).pack(padx=10,pady=10)
tk.Button(root, text="Extrair identificador", width=30, command=extrair).pack(padx=10,pady=10)
root.mainloop()
