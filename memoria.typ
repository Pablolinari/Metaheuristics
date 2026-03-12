// Configuración general del documento
#set page(
  paper: "a4",
  margin: (x: 3cm, y: 2.5cm),
  numbering: "1",
)

#set text(
  font: "FreeSans",
  size: 11pt,
  lang: "es"
)
#set heading(numbering: "1.1")
// --- PORTADA ---
#align(center)[
  #block(text(weight: "bold", 2em)[Práctica 1 \ Problema del portfolio])
  #v(1em)
  #text(size: 1.2em)[Pablo Linari Pérez]
  #v(2em)
]

// --- ÍNDICE AUTOMÁTICO ---
// Se actualizará solo cada vez que guardes el archivo
#outline(
  title: [Índice de contenidos],
  indent: auto,
)

#pagebreak()

// --- CONTENIDO ---

= Introducción
Este es el primer apartado. Typst lo añadirá al índice automáticamente.

== Antecedentes
Al ser un título de nivel 2, aparecerá indentado en el índice.

= Desarrollo del Proyecto
Puedes seguir escribiendo y, al añadir un nuevo título de nivel 1 como este, verás la magia en el panel de previsualización.

== Metodología
Aquí va tu explicación técnica. 

= Conclusión
El documento está listo para crecer.
