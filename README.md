# Computação Gráfica - UFABC
## Atividade 3 - Aplicação interativa com gráficos 3D, iluminação e texturização

Link WebAssembly: 
Link para o vídeo do Youtube: https://www.youtube.com/watch?v=kAWeEBukILc

### Integrantes:
Nomes:
- Fernando Schroder
- Gabriel Yuto Munakata

Registros Acadêmico:
- 11201921885
- 11201721879

### Descrição da aplicação
O objetivo desta aplicação é criar um cenário de um labirinto em cubos, de modo que existem cubos dourados dentro deste labirito que precisam ser encontrados.


<div align="center">

![Aplicação](./images/image3.jpeg)

</div>

A aplicação teve como base o projeto LookAt apresentado durante as aulas de laboratório. Dessa forma, foi construído um cenário em que é gerado um "solo" com base na classe ground.cpp e ground.hpp, e também é construído a classe camera.cpp e camera.hpp para se obter uma visão do espaço da camera a partir do espaço do mundo.

### Implementação
Para a implementação, primeiro foi feita uma modificação dos valores de core RGB do solo, atribuindo a cor da seguinte forma:

```c++
abcg::glUniform4f(m_colorLoc, 1.0, 0.0, 1.0, 1.0f);
```

Ja em relação a construção do cenário com cubos, simulando o labirinto, utilizamos dentro do método onPaint() alguns loops para renderizar o box.obj importado da pasta assets.
Para a posicionamento dos cubos, foi a seguinte implementaçã0:

```c++
  const int limit_sup{50};
  const int limit_inf{-50};
  
  for (int x = limit_inf; x < limit_sup; x++) {
    for (int z = limit_inf; z < limit_sup; z++) {
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(x, 0.5f, z));
      model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(0.5f));

      abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(m_colorLocation, x, (x + z), z, 1.0f);
      abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);                  
    }
  }
```

O primeiro loop descreve a posição no eixo x que iremos colocar os cubos, e o segundo loop descreve a posição no eixo z. Com base nesse vetor (x,z), temos as coordenadas da posição dos cubos, no qual é transladado por meio do método glm::translate.
Adicionamos uma pequena rotação para criar um efeito mais interessante para a aplicação. E por fim, ajustamos a escala do objeto por meio do método glm::rotate e atribuimos uma cor ao cubo.

Um esquema detalhando a contrução do cenário é descrito na figura abaixo:


<div align="center">

![Aplicação](./images/image2.png)

</div>

Tendo como base a geração dos primeiros cubos, repetimos o mesmo loop para gerar cubos acima dos mesmos, de modo a se construir paredes de cubos. A diferença se encontra no posicionamento no eixo y, a rotação dos objetos e a cor atribuido.


Por fim, existe um loop for que gera cubos dourados em diversas direções dentro do cenário.
A exemplificação de como ocorre essa geração e indicada no código abaixo:


```c++
const int count_gold_cube{10};

  for (int a = 0; a < count_gold_cube; a++) {
    for(int b = 0; b  < count_gold_cube; b++){
      glm::mat4 model{1.0f};
      model = glm::translate(model, glm::vec3(a/2, 0.5f, b/2));
      model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
      model = glm::scale(model, glm::vec3(0.1f));

      abcg::glUniformMatrix4fv(m_modelMatrixLocation, 1, GL_FALSE, &model[0][0]);
      abcg::glUniform4f(m_colorLocation, 255, 223, 0.0, 1.0f);
      abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }
  }
````

Explicações das funções por arquivo do repositório:

1. camera.cpp e camera.hpp

Estes arquivos contêm a implementação e declaração de uma classe de câmera, que é usada para controlar a visão do jogador no jogo. Funções comuns incluem:

    Inicialização da câmera.
    Ajuste da posição e orientação da câmera.
    Métodos para manipular a visualização, como zoom, rotação e translação.

2. main.cpp

Este é o ponto de entrada do programa. Normalmente, contém:

    A função main, que inicializa o jogo e inicia o loop principal.
    Configurações iniciais do jogo, como criação de janelas e inicialização de subsistemas.

3. maze.cpp e maze.hpp

Estes arquivos são responsáveis pela lógica e representação do labirinto no jogo.

    Funções para gerar o layout do labirinto.
    Métodos para desenhar o labirinto na tela.
    Lógica para detectar colisões ou interações dentro do labirinto.

4. model.cpp e model.hpp

Esses arquivos lidam com a representação de modelos 3D no jogo. Funções comuns incluem:

    Carregamento de modelos a partir de arquivos.
    Renderização de modelos na tela.
    Manipulação de propriedades dos modelos, como posição, escala e rotação.

5. window.cpp e window.hpp

Estes arquivos são responsáveis pela criação e gerenciamento da janela do jogo. Eles podem incluir:

    Funções para criar e configurar a janela do jogo.
    Loop de eventos para processar entradas do usuário e atualizações de tela.
    Métodos para lidar com eventos de janela, como redimensionamento ou fechamento.

Vou verificar o conteúdo específico de cada arquivo para fornecer detalhes mais precisos. ​

​GoldCubesGame:
camera.cpp

    initializeCamera(Maze maze): Inicializa a câmera com o labirinto.
    computeViewMatrix(): Calcula a matriz de visualização da câmera.
    computeProjectionMatrix(glm::vec2 const &size): Calcula a matriz de projeção da câmera.
    dolly(float speed): Move a câmera para frente/trás.
    truck(float speed): Move a câmera para esquerda/direita.
    pan(float speed): Rotaciona a câmera.

main.cpp

    main(int argc, char **argv): Função principal do programa.

maze.cpp

    initializeMaze(std::string path): Inicializa o labirinto com um caminho específico.

model.cpp

    loadObj(std::string_view path, bool standardize): Carrega um modelo OBJ.
    loadDiffuseTexture(std::string_view path): Carrega a textura difusa do modelo.
    loadNormalTexture(std::string_view path): Carrega a textura normal do modelo.
    createBuffers(): Cria buffers para o modelo.
    standardize(): Padroniza o modelo.
    computeNormals(): Calcula as normais do modelo.
    computeTangents(): Calcula as tangentes do modelo.
    render() const: Renderiza o modelo.
    setupVAO(GLuint program): Configura o Vertex Array Object do modelo.


window.cpp

    onEvent(SDL_Event const &event): Lida com eventos da SDL.
    onCreate(): Chamada ao criar a janela.
    onPaint(): Chamada para pintar na janela.
    onPaintUI(): Chamada para pintar a interface do usuário.
    onResize(glm::ivec2 const &size): Chamada quando a janela é redimensionada.
    onUpdate(): Chamada para atualizar a janela.
    onDestroy(): Chamada ao destruir a janela.
    renderMaze(): Renderiza o labirinto.
    renderBox(): Renderiza uma caixa.
    renderTrophy(): Renderiza um troféu.
