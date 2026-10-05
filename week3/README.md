# Week 3

Үш вершинаға әртүрлі RGB түс берілгенде олардың арасында градиент пайда болады. Үш вершинаға бірдей түс бергенде градиент жоғалып, үшбұрыш бір түсті болды.

Fragment shader-де 1.0 - vColor арқылы түстер кері мәнге өзгертілді. Vertex shader-де gl_Position-ның y мәні -1-ге көбейтіліп, үшбұрыш төңкерілді.

Shader-ге әдейі vec4 орнына vec5 қатесі енгізілді. Консольде келесі қате шықты:

Shader error:
0:5(10): error: syntax error, unexpected NEW_IDENTIFIER, expecting '{'

Link error:
error: linking with uncompiled/unspecialized shader

Соңғы нұсқада қате түзетіліп, vec4 қайта қойылды.
