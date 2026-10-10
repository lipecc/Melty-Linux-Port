/* SPDX-License-Identifier: MIT
 * Localization for Melty Linux Port's own native installer and downloader.
 * The upstream melty.gg website is maintained separately.
 */
#ifndef MELTY_LINUX_I18N_H
#define MELTY_LINUX_I18N_H
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

enum { ML_EN, ML_PT, ML_ES, ML_FR, ML_DE, ML_IT, ML_JA, ML_LANG_COUNT };
enum {
  M_INSTALL_TITLE, M_UPDATE_TITLE, M_INSTALL_HELP, M_UPDATE_HELP,
  M_INSTALL_PATH, M_INSTALL_SHORTCUT, M_INSTALL_OAUTH, M_INSTALL_NOPASS,
  M_INSTALL_BUTTON, M_UPDATE_BUTTON, M_RUN_BUTTON, M_CANCEL,
  M_LANG, M_DOWN_TITLE, M_DOWN_HELP, M_CONNECT, M_CONNECT_ELECTRON,
  M_DOWNLOADING, M_DOWNLOADING_PERCENT, M_DOWN_PREPARE, M_MIRROR,
  M_CANCELLED, M_FINISHED, M_EXTRACT_NEXT, M_DOWNLOAD_FAILED,
  ML_MESSAGE_COUNT
};
static const char *const ml_codes[ML_LANG_COUNT] = {
  "en", "pt-BR", "es", "fr", "de", "it", "ja"
};
static const char *const ml_names[ML_LANG_COUNT] = {
  "English", "Português", "Español", "Français", "Deutsch", "Italiano", "日本語"
};
static const char *const ml_text[ML_LANG_COUNT][ML_MESSAGE_COUNT] = {
 { /* English */
  "Install Melty for Linux", "Update Melty", "Set up Melty for your Linux desktop.",
  "An updated AppImage is ready to install.",
  "Install to ~/.local/share/Melty/Melty.AppImage",
  "Add an app-menu shortcut and GNOME/KDE icon",
  "Register Google sign-in links (melty://)",
  "Administrator access is not required.",
  "INSTALL & OPEN", "UPDATE & OPEN", "RUN ONLY", "CANCEL", "Language",
  "Preparing Melty", "Installing the required components",
  "Connecting to the server...", "Connecting to the Electron server...",
  "Downloading Electron 39.8.10 - %.1f MB",
  "Downloading Electron 39.8.10 - %.1f MB (%d%%)",
  "Preparing Melty components", "Main server unavailable. Trying mirror...",
  "Download cancelled", "Download complete!",
  "Melty will extract Electron and start.", "Check your connection and the launcher log."
 },
 { /* Brazilian Portuguese */
  "Instalar Melty no Linux", "Atualizar Melty", "Configure o Melty no seu desktop Linux.",
  "Uma nova versão AppImage está pronta para instalar.",
  "Instalar em ~/.local/share/Melty/Melty.AppImage",
  "Criar atalho no menu e ícone no GNOME/KDE",
  "Registrar links de login Google (melty://)",
  "Não é necessário usar administrador.",
  "INSTALAR E ABRIR", "ATUALIZAR E ABRIR", "SÓ EXECUTAR", "CANCELAR", "Idioma",
  "Preparando o Melty", "Instalando os componentes necessários",
  "Conectando ao servidor...", "Conectando ao servidor do Electron...",
  "Baixando Electron 39.8.10 - %.1f MB",
  "Baixando Electron 39.8.10 - %.1f MB (%d%%)",
  "Preparando os componentes do Melty", "Servidor indisponível. Tentando espelho...",
  "Download cancelado", "Download concluído!",
  "O Melty vai extrair o Electron e iniciar.", "Verifique a conexão e o log de inicialização."
 },
 { /* Spanish */
  "Instalar Melty en Linux", "Actualizar Melty", "Configura Melty en tu escritorio Linux.",
  "Hay una actualización AppImage lista para instalar.",
  "Instalar en ~/.local/share/Melty/Melty.AppImage",
  "Crear acceso directo e icono GNOME/KDE",
  "Registrar enlaces de acceso Google (melty://)",
  "No se requieren permisos de administrador.",
  "INSTALAR Y ABRIR", "ACTUALIZAR Y ABRIR", "SOLO EJECUTAR", "CANCELAR", "Idioma",
  "Preparando Melty", "Instalando los componentes necesarios",
  "Conectando al servidor...", "Conectando al servidor de Electron...",
  "Descargando Electron 39.8.10 - %.1f MB",
  "Descargando Electron 39.8.10 - %.1f MB (%d%%)",
  "Preparando los componentes de Melty", "Servidor principal no disponible. Otro servidor...",
  "Descarga cancelada", "¡Descarga completa!",
  "Melty extraerá Electron y se iniciará.", "Comprueba tu conexión y el registro."
 },
 { /* French */
  "Installer Melty sur Linux", "Mettre à jour Melty", "Configurez Melty pour votre bureau Linux.",
  "Une mise à jour AppImage est prête à installer.",
  "Installer dans ~/.local/share/Melty/Melty.AppImage",
  "Ajouter un raccourci et une icône GNOME/KDE",
  "Enregistrer les liens Google (melty://)",
  "Aucun droit administrateur nécessaire.",
  "INSTALLER ET OUVRIR", "METTRE À JOUR", "EXÉCUTER", "ANNULER", "Langue",
  "Préparation de Melty", "Installation des composants nécessaires",
  "Connexion au serveur...", "Connexion au serveur Electron...",
  "Téléchargement Electron 39.8.10 - %.1f Mo",
  "Téléchargement Electron 39.8.10 - %.1f Mo (%d%%)",
  "Préparation des composants Melty", "Serveur principal indisponible. Autre miroir...",
  "Téléchargement annulé", "Téléchargement terminé !",
  "Melty va extraire Electron et démarrer.", "Vérifiez votre connexion et le journal."
 },
 { /* German */
  "Melty für Linux installieren", "Melty aktualisieren", "Melty für deinen Linux-Desktop einrichten.",
  "Ein aktualisiertes AppImage ist bereit.",
  "Installieren: ~/.local/share/Melty/Melty.AppImage",
  "App-Menü-Eintrag und GNOME/KDE-Icon anlegen",
  "Google-Anmeldelinks registrieren (melty://)",
  "Keine Administratorrechte nötig.",
  "INSTALLIEREN", "AKTUALISIEREN", "NUR STARTEN", "ABBRECHEN", "Sprache",
  "Melty wird vorbereitet", "Benötigte Komponenten werden installiert",
  "Serververbindung wird aufgebaut...", "Verbindung zum Electron-Server...",
  "Electron 39.8.10 wird geladen - %.1f MB",
  "Electron 39.8.10 wird geladen - %.1f MB (%d%%)",
  "Melty-Komponenten werden vorbereitet", "Hauptserver nicht erreichbar. Teste Spiegel...",
  "Download abgebrochen", "Download abgeschlossen!",
  "Melty entpackt Electron und startet.", "Verbindung und Protokoll prüfen."
 },
 { /* Italian */
  "Installa Melty su Linux", "Aggiorna Melty", "Configura Melty sul tuo desktop Linux.",
  "Un AppImage aggiornato è pronto da installare.",
  "Installa in ~/.local/share/Melty/Melty.AppImage",
  "Crea collegamento e icona GNOME/KDE",
  "Registra i link di accesso Google (melty://)",
  "Non sono necessari privilegi amministrativi.",
  "INSTALLA E APRI", "AGGIORNA E APRI", "SOLO AVVIA", "ANNULLA", "Lingua",
  "Preparazione di Melty", "Installazione dei componenti necessari",
  "Connessione al server...", "Connessione al server Electron...",
  "Download Electron 39.8.10 - %.1f MB",
  "Download Electron 39.8.10 - %.1f MB (%d%%)",
  "Preparazione dei componenti Melty", "Server principale non disponibile. Provo il mirror...",
  "Download annullato", "Download completato!",
  "Melty estrarrà Electron e si avvierà.", "Controlla la connessione e il registro."
 },
 { /* Japanese */
  "Linux に Melty をインストール", "Melty をアップデート", "Linux デスクトップ用に Melty を設定します。",
  "新しい AppImage をインストールできます。",
  "保存先: ~/.local/share/Melty/Melty.AppImage",
  "アプリ一覧にショートカットとアイコンを追加",
  "Google ログインのリンクを登録 (melty://)",
  "管理者権限は必要ありません。",
  "インストールして起動", "更新して起動", "今回のみ実行", "キャンセル", "言語",
  "Melty を準備中", "必要なコンポーネントをインストールしています",
  "サーバーに接続しています...", "Electron サーバーに接続しています...",
  "Electron 39.8.10 ダウンロード中 - %.1f MB",
  "Electron 39.8.10 ダウンロード中 - %.1f MB (%d%%)",
  "Melty のコンポーネントを準備中", "接続できません。ミラーを試しています...",
  "ダウンロードを中止しました", "ダウンロード完了！",
  "Electron を展開して Melty を起動します。", "接続とログを確認してください。"
 }
};
static int ml_from_code(const char *code) {
  if (!code || !*code) return -1;
  char a=(char)tolower((unsigned char)code[0]);
  char b=(char)tolower((unsigned char)code[1]);
  if (a=='p' && b=='t') return ML_PT;
  if (a=='e' && b=='s') return ML_ES;
  if (a=='f' && b=='r') return ML_FR;
  if (a=='d' && b=='e') return ML_DE;
  if (a=='i' && b=='t') return ML_IT;
  if (a=='j' && b=='a') return ML_JA;
  if (a=='e' && b=='n') return ML_EN;
  return -1;
}
static int ml_detect(void) {
  const char *names[]={"MELTY_LANG","LC_ALL","LC_MESSAGES","LANGUAGE","LANG"};
  for (size_t i=0;i<sizeof(names)/sizeof(names[0]);i++) {
    const char *v=getenv(names[i]);
    int l=ml_from_code(v);
    if(l>=0) return l;
  }
  return ML_EN;
}
static const char *ml_get(int lang,int index) {
  if(lang<0||lang>=ML_LANG_COUNT||index<0||index>=ML_MESSAGE_COUNT) return "";
  return ml_text[lang][index];
}
#endif
