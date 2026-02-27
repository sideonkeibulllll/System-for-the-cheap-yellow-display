# YIYAN-OS

**Un Framework de Système GUI Embarqué Haute Performance pour ESP32 "Cheap Yellow Display" (ESP32-2432S028R)**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: LVGL](https://img.shields.io/badge/Framework-LVGL%20v8.4-green.svg)](https://lvgl.io/)

[中文](README.zh-CN.md) | [English](README.md) | [Français](README.fr.md)

---

## Aperçu

YIYAN-OS est un framework complet de système GUI embarqué conçu pour la carte de développement ESP32-2432S028R (Cheap Yellow Display / CYD). Basé sur quatre principes de conception fondamentaux : **Utilisation Maximale des Ressources**, **Optimisation Intelligente**, **Configuration de Type BIOS**, et **Interface Matérielle Simplifiée**, il fournit une base robuste, efficace et flexible.

## ✨ Caractéristiques Principales

### 🖥️ Moteur Graphique Haute Performance
- **Intégration LVGL v8.4** : Bibliothèque graphique légère de référence
- **Traitement Parallèle Double-Cœur** : Cœur 1 pour le rendu UI, Cœur 0 pour la logique applicative
- **Transfert DMA Double Buffer** : Élimine le déchirement d'image, atteint 60 FPS
- **Optimisation du Rafraîchissement Partiel** : Redessine uniquement les zones modifiées

### 🔧 Système de Configuration Type BIOS
- **Chargement de Configuration à Trois Niveaux** : Défauts du code → Persistance NVS → Fichiers config SD
- **Ajustement des Paramètres à l'Exécution** : Modifier le comportement sans recompilation
- **Découplage Matériel** : Changer le matériel en modifiant simplement les fichiers de configuration

### 📱 Framework de Gestion Multi-Applications
- **Gestion du Cycle de Vie des Applications** : Active (premier plan), Paused (arrière-plan), Stopped (inactif)
- **Récupération Intelligente de la Mémoire** : Les applications en arrière-plan libèrent automatiquement les objets LVGL
- **Basculement Fluide des Applications** : Mécanisme de préchargement éliminant les délais

### ⚡ Gestion Intelligente de l'Énergie
- **Contrôle Automatique du Rétroéclairage** :
  - Mode Manuel : Luminosité réglable par l'utilisateur
  - Mode Auto : Ajustement automatique selon la lumière ambiante (nécessite LDR)
  - Mode Off : Rétroéclairage éteint
- **Gestion de l'État Inactif** : Entre en état Idle après 30 secondes d'inactivité
- **Sommeil Profond** : Light Sleep après 5 minutes, réveil par toucher ou bouton

### 💾 Intégration du Système de Stockage
- **Système de Fichiers SPIFFS** : Stockage Flash intégré pour les ressources système
- **Extension Carte SD** : Grande capacité pour les données utilisateur et médias
- **Interface de Fichiers LVGL** : Accès unifié à `F:` (Flash) et `S:` (Carte SD)

### 🎛️ Couche d'Abstraction Matérielle Complète
- **Affichage** : Pilote ILI9341 / ST7789, résolution 320×240
- **Écran Tactile** : Touche résistive XPT2046 avec support de calibration
- **LED RGB** : Indicateur tricolore intégré
- **Capteur de Lumière** : Détection de lumière ambiante (GPIO 34)
- **Bouton BOOT** : Réveil/commutation multi-fonction

## 🤖 Applications Intégrées

### 💬 ChatApp - Chat IA
Supporte plusieurs API de modèles de langage IA :
- **DeepSeek API** : Supporte le modèle deepseek-chat
- **GLM-5 API** : Supporte le modèle GLM-5 de Zhipu AI
- **SiliconFlow API** : Supporte divers modèles open-source (Qwen, DeepSeek-R1, etc.)
- **Saisie Chinoise** : Méthode de saisie Ziranma intégrée
- **Réponse en Flux** : Affichage en temps réel des réponses IA
- **Historique** : Visualisation de l'historique des conversations

Configuration : Copiez `src/api_config_example.h` vers `src/api_config.h` et ajoutez votre clé API :
```cpp
#define API_KEY "your-api-key-here"
#define API_BASE_URL "https://api.deepseek.com"
#define API_MODEL "deepseek-chat"
```

### 📁 FileExplorerApp - Gestionnaire de Fichiers
- **Double Stockage** : Navigation dans les fichiers SPIFFS et carte SD
- **Opérations sur Fichiers** : Visualisation, copie, déplacement, suppression
- **Aperçu d'Images** : Support des formats BMP, JPG, PNG
- **Visionneuse de Texte** : Navigation dans les fichiers texte

### ⚙️ SettingsApp - Paramètres Système
- **Contrôle du Rétroéclairage** : Modes Manuel/Auto/Off
- **Configuration WiFi** : Scan et connexion aux réseaux sans fil
- **Informations Système** : Visualisation de l'utilisation mémoire, temps de fonctionnement, etc.

### 📶 WiFiConfigApp - Configuration WiFi
- **Scan des Réseaux** : Scan automatique des réseaux WiFi environnants
- **Saisie du Mot de Passe** : Clavier virtuel pour la saisie du mot de passe
- **État de Connexion** : Affichage en temps réel de l'état de connexion et de l'adresse IP

### 🎨 DemoApp - Application de Démo
- **Présentation UI** : Démonstration des effets des contrôles LVGL
- **Test de Performance** : Test des performances de rendu du système

### 🔤 FontApp - Gestion des Polices
- **Navigation des Polices** : Visualisation des polices disponibles
- **Aperçu des Polices** : Prévisualisation du rendu des polices

## 📊 Indicateurs de Performance

| Indicateur | Valeur |
|------------|--------|
| Taux de Rafraîchissement (FPS) | ~60 |
| Utilisation Mémoire | ~105 Ko (LVGL + Système) |
| Utilisation Flash | ~800 Ko |
| Latence de Réponse UI | < 50 ms |
| Temps de Basculement App | < 200 ms |

## 🚀 Démarrage Rapide

### Configuration Matérielle Requise
- Carte de développement ESP32-2432S028R (Cheap Yellow Display)
- Carte microSD (optionnel)
- Câble de données USB

### Environnement Logiciel
- PlatformIO IDE (recommandé) ou Arduino IDE
- Package de support carte ESP32 v3.0+

### Compilation et Flashage
```bash
# Cloner le projet
git clone <repository-url>

# Compiler avec PlatformIO
pio run

# Flasher le firmware
pio run --target upload
```

### Configuration API (Optionnel)
Pour utiliser la fonctionnalité de chat IA, copiez le fichier de configuration :
```bash
cp src/api_config_example.h src/api_config.h
```
Puis modifiez `src/api_config.h` et ajoutez votre clé API.

## 📁 Structure du Projet

```
├── src/
│   ├── main.cpp           # Point d'entrée principal
│   ├── ConfigManager.*    # Gestion de configuration
│   ├── BSP.*              # Package de support carte
│   ├── Storage.*          # Système de stockage
│   ├── Performance.*      # Surveillance des performances
│   ├── PowerManager.*     # Gestion de l'énergie
│   ├── AppManager.*       # Gestionnaire d'applications
│   ├── ChatApp.*          # Application de chat IA
│   ├── FileExplorerApp.*  # Gestionnaire de fichiers
│   ├── WiFiConfigApp.*    # Configuration WiFi
│   ├── SettingsApp.*      # Paramètres système
│   ├── DemoApp.*          # Application démo
│   ├── FontApp.*          # Gestion des polices
│   ├── ZhFont.*           # Support des polices chinoises
│   ├── LvZhFont.*         # Rendu chinois LVGL
│   ├── ZhDraw.*           # Composant de dessin chinois
│   └── lv_conf.h          # Configuration LVGL
├── include/
├── lib/
└── platformio.ini
```

## 🔌 Référence API

### Développement d'Application

```cpp
// Enregistrer une nouvelle application
AppMgr.registerApp("MonApp", createMonApp, &appInfo);

// Basculer vers l'application
AppMgr.switchToApp("MonApp");

// Retourner à l'écran d'accueil
AppMgr.switchToHome();

// Définir la luminosité du rétroéclairage (0-255)
Power.setBacklight(128);

// Définir le mode de rétroéclairage
Power.setBacklightMode(BACKLIGHT_MODE_AUTO);
```

### Cycle de Vie de l'Application

```cpp
class MonApp : public BaseApp {
    bool createUI() override;    // Créer l'UI
    void onDestroy() override;   // Nettoyage
    bool onResume() override;    // Reprendre au premier plan
    void onPause() override;     // Basculer en arrière-plan
    void onUpdate() override;    // Mise à jour périodique
};
```

## 📝 Journal des Modifications

**v3.0.0** - Mise à Jour Majeure
- Optimisation de la fonctionnalité de chat IA ChatApp
- Amélioration des performances de rendu chinois
- Renforcement de la stabilité du système

**Série v2.x** - Amélioration des Fonctionnalités
- Ajout du support de chat IA
- Optimisation du rendu des polices chinoises
- Amélioration du gestionnaire de fichiers
- Ajout de la surveillance des performances

## 📄 Licence

MIT License

## 🤝 Contribution

Les contributions sont les bienvenues ! N'hésitez pas à soumettre une Pull Request.

## 📧 Contact

Pour toute question ou suggestion, veuillez ouvrir un Issue sur le dépôt.

---

**YIYAN-OS** - *Permettre le Développement GUI Embarqué*
