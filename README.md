 Editor e simulador visual de Extended Finite State Machines (EFSM) em Qt (Widgets + Graphics View). 
Suporta: criação de estados e transições, guardas e ações (JavaScript via QJSEngine), variáveis (X), 
entradas (I), saídas (O), setas curvas para paralelas e bidirecionais, múltiplos self-loops organizados, 
e persistência em JSON com “Salvar…”/“Abrir…”.

────────────────────────────────────────────────────────────────────────────
1) Visão Geral
────────────────────────────────────────────────────────────────────────────
O EFSMStudio foi projetado para que o usuário construa e simule EFSMs de forma visual. 
Principais capacidades:
• Adicionar estados (marcar inicial/final; renomear por duplo clique; estado “ativo” é realçado).
• Criar transições arrastando do estado de origem ao destino (inclui self-loop).
• Editar cada transição: rótulo, prioridade, guarda g(X,I) e ação a(X,I,O).
• Painéis laterais para gerenciar X (variáveis), I (inputs) e O (outputs).
• Executar um passo (Step/F10): avalia guardas, escolhe transição (menor prioridade, empate por id)
  e executa ações, atualizando X/O.
• Visualização legível de:
  – Paralelas na mesma direção (curvas distribuídas).
  – Bidirecionais (x→y e y→x) em lados opostos.
  – Múltiplos self-loops por estado sem sobreposição de rótulos.
• Persistência do modelo via JSON (com extensão “.json” aplicada automaticamente ao salvar).

────────────────────────────────────────────────────────────────────────────
2) Requisitos
────────────────────────────────────────────────────────────────────────────
• CMake ≥ 3.16
• C++17
• Qt 5.15+ ou Qt 6.x (módulos: Core, Widgets, Qml)
• Compilador compatível (GCC/Clang/MSVC)
• Linux, macOS ou Windows

────────────────────────────────────────────────────────────────────────────
3) Estrutura do Código (arquivos principais)
────────────────────────────────────────────────────────────────────────────
main.cpp                      // ponto de entrada (QApplication + MainWindow)
MainWindow.h/.cpp             // janela principal, toolbar, docks (X/I/O), step, salvar/abrir
DiagramScene.h/.cpp           // cena do canvas: modos Select/AddTransition e criação de arestas
StateItem.h/.cpp              // nós/estados: círculo, inicial/final/ativo, rename
TransitionItem.h/.cpp         // arestas: Bezier, cabeça de seta, paralelas, bidirecionais, self-loops
TransitionEditorDialog.h/.cpp // diálogo de edição de transição
VarModel.h/.cpp               // modelo de tabela para Variáveis (X)
InputModel.h/.cpp             // modelo de tabela para Inputs (I)
OutputModel.h/.cpp            // modelo de tabela para Outputs (O)

Observação: O diretório tests/ e quaisquer artefatos de teste foram removidos do projeto por opção.

────────────────────────────────────────────────────────────────────────────
4) Como Compilar
────────────────────────────────────────────────────────────────────────────
No diretório do projeto:
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
  cmake --build build -j

Executar:
  • Linux/macOS: ./build/EFSMStudio
  • Windows:     build\Release\EFSMStudio.exe  (ou Debug\EFSMStudio.exe)

────────────────────────────────────────────────────────────────────────────
5) Como Usar (Fluxo Básico)
────────────────────────────────────────────────────────────────────────────
1. Criar estados:
   – Botão “Novo Estado” na toolbar cria S2, S3… (S1 é criado automaticamente no início).
   – Duplo clique no estado para renomear.
   – “Marcar Inicial” (Ctrl+I) define o inicial (borda azul).
   – “Alternar Final” (Ctrl+F) adiciona o anel interno do estado final.
2. Criar transições:
   – Ative “Transição” na toolbar.
   – Clique no estado de origem e solte sobre o destino.
   – Para self-loop, solte de volta no próprio estado.
   – O editor abre: defina rótulo, prioridade, guarda e ação.
3. Editar transições existentes:
   – Selecione a transição e clique “Editar Transição”.
4. Gerenciar X/I/O:
   – Nos docks à direita, use “Adicionar/Excluir” e edite inline nome/valor.
   – Valores aceitos: true/false, inteiros e strings (bool exibido como “true/false”).
5. Executar um passo (Step/F10):
   – Guarda g(X,I) é avaliada em JavaScript (QJSEngine); 
     se vazia, assume “true”.
   – Entre as habilitadas, escolhe menor prioridade; desempate pelo id (mais antiga).
   – Ação a(X,I,O) é avaliada após substituir “:=” por “=”.
   – Valores de X e O são atualizados na UI; inputs I permanecem.
   – O estado corrente muda para o destino da transição escolhida.
6. Salvar/Abrir:
   – “Salvar…”: caixa sugere “*.json” automaticamente (DefaultSuffix).
   – “Abrir…”: carrega o JSON e reconstrói cena e tabelas.

────────────────────────────────────────────────────────────────────────────
6) Semântica de Simulação (Detalhes)
────────────────────────────────────────────────────────────────────────────
• Estado corrente: inicial ao primeiro Step, se ainda não definido.
• Candidatas: transições cuja origem == estado corrente.
• Guardas: JS no escopo com variáveis X, I e O.
  – Erro na avaliação ⇒ guarda tratada como falsa (mensagem na barra de status).
• Escolha: menor prioridade; empate por id (ordem de criação).
• Ações: JS após “:=”→“=”. Em caso de erro, o step é abortado para evitar inconsistência.
• Atualização: leitura de volta das variáveis X e O do contexto JS para as tabelas.
• Avanço: desmarca estado anterior como “ativo” e marca o destino.

────────────────────────────────────────────────────────────────────────────
7) Geometria e Legibilidade das Transições
────────────────────────────────────────────────────────────────────────────
• Arestas são Beziers cúbicas com cabeça de seta triangular.
• Paralelas (mesma direção x→y): separação centrada (…, −1, 0, +1 …) para curvas distintas.
• Bidirecionais (x→y e y→x): cada direção curva para um lado oposto; magnitude 0.5, 1.5, 2.5… 
  garantindo separação simétrica e fácil leitura.
• Self-loops:
  – Distribuição angular por índice (evita sobreposição).
  – “Camadas” mais externas quanto mais loops existirem (loopOut crescente).
  – Rótulos: posicionados alinhados ao centro do estado e abaixo do arco do próprio loop,
    minimizando sobreposição entre textos.

────────────────────────────────────────────────────────────────────────────
8) Persistência (Formato JSON)
────────────────────────────────────────────────────────────────────────────
• Estrutura:
  { "vars":    [ {"name": "...", "value": ...}, ... ],
    "inputs":  [ {"name": "...", "value": ...}, ... ],
    "outputs": [ {"name": "...", "value": ...}, ... ],
    "states":  [ {"name":"S1","x":0,"y":0,"initial":true,"final":false}, ... ],
    "transitions": [
      {"from":"S1","to":"S2","guard":"btn && credit>0",
       "action":"dispense:=true; credit:=credit-1",
       "priority":1, "label":"vend"}
    ]
  }
• Tipagem de value: bool/number/string; conversão automática ao salvar/carregar.
• Ordem das transições: persistida por id de criação para manter estética (paralelas/self-loops).
• Salvar: diálogo aplica “.json” automaticamente (setDefaultSuffix("json")).

────────────────────────────────────────────────────────────────────────────
9) CMake
────────────────────────────────────────────────────────────────────────────
Exemplo mínimo de CMakeLists.txt (Qt5/Qt6):
----------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)
project(EFSMStudio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Qml)
if (QT_VERSION_MAJOR EQUAL 6)
  find_package(Qt6 REQUIRED COMPONENTS Core Widgets Qml)
else()
  find_package(Qt5 REQUIRED COMPONENTS Core Widgets Qml)
endif()

add_library(EFSMStudioLib
    MainWindow.h MainWindow.cpp
    StateItem.h StateItem.cpp
    VarModel.h VarModel.cpp
    InputModel.h InputModel.cpp
    OutputModel.h OutputModel.cpp
    TransitionEditorDialog.h TransitionEditorDialog.cpp
    TransitionItem.h TransitionItem.cpp
    DiagramScene.h DiagramScene.cpp
)

target_link_libraries(EFSMStudioLib PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
)

add_executable(EFSMStudio
    main.cpp
)

target_link_libraries(EFSMStudio PRIVATE
    EFSMStudioLib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
)
----------------------------------------------------------------

────────────────────────────────────────────────────────────────────────────
10) Solução de Problemas
────────────────────────────────────────────────────────────────────────────
• “QApplication: No such file or directory” → Instale pacotes de desenvolvimento do Qt e garanta 
  que o componente Widgets foi encontrado no find_package; inclua <QApplication> no main.cpp.
• Erros de link com Qt → Confira target_link_libraries com Qt::Core, Qt::Widgets, Qt::Qml.
• JSON inválido ao abrir → Verifique chaves obrigatórias e tipos; mensagens aparecerão em QMessageBox.

────────────────────────────────────────────────────────────────────────────
11) Decisões de Engenharia
────────────────────────────────────────────────────────────────────────────
• Graphics View (QGraphicsScene/QGraphicsView) para interatividade e performance.
• Bezier + offsets controlados → diferença visual clara com custo baixo.
• Ids crescentes em TransitionItem → desempate determinístico e persistência estável.
• QJSEngine → evita DSL própria e aproveita um runtime maduro.
• Modelos de tabela (Var/Input/Output) simples e consistentes (parse bool/int/string).

────────────────────────────────────────────────────────────────────────────
12) Desempenho
────────────────────────────────────────────────────────────────────────────
• Antialiasing habilitado na QGraphicsView.
• Recalcula paths apenas quando necessário (movimento de estado, criação/remoção de aresta, etc.).
• Suporta cenas médias com boa fluidez (setas simples + Bezier).

────────────────────────────────────────────────────────────────────────────
13) Limitações e Roadmap
────────────────────────────────────────────────────────────────────────────
Limitações:
• Sem Undo/Redo.
• Sem auto-layout de grafos.
• Sem validação estática de guardas/ações (apenas runtime).

Roadmap sugerido:
• Undo/Redo (QUndoStack).
• Auto-layout (algoritmos de grafos).
• Exportação para imagem/PDF.
• Lint simples para guardas/ações JS e destaques na UI.
• Temas (claro/escuro) e atalhos configuráveis.

────────────────────────────────────────────────────────────────────────────
14) Licença e Créditos
────────────────────────────────────────────────────────────────────────────
• Licença: definir (MIT/BSD/GPL, etc.).
• Autores: Eduardo Silveira Godinho & Sergio Bonini.
• Tecnologias: Qt, CMake.
