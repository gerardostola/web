// Función reutilizable para cargar contenido dinámico
function cargarContenido(id, contenedorId) {
    fetch(`dyn?id=${id}`)
        .then(response => {
            if (!response.ok) {
                throw new Error('Error');
            }
            return response.text();
        })
        .then(data => {
            // Insertar el contenido en el contenedor
            document.getElementById(contenedorId).innerHTML = data;
        })
        .catch(error => {
            console.error('Error:', error);
            document.getElementById(contenedorId).innerHTML = '<p>Error al cargar el contenido.</p>';
        });
}

// Función reutilizable para enviar datos de un formulario mediante POST
function enviarFormulario(formularioId, url, contenedorRespuestaId) {
    // Obtener el formulario
    const formulario = document.getElementById(formularioId);

    // Crear un objeto para almacenar los datos
    const datos = {};
    const elementos = formulario.querySelectorAll('input, select, textarea');

    elementos.forEach(elemento => {
        if (elemento.id) {
            datos[elemento.id] = elemento.value; // Usa el id como clave y el valor del campo como valor
        }
    });

    // Realizar el POST con los datos recolectados
    fetch(url, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(datos), // Enviar los datos como JSON
    })
        .then(response => {
            if (!response.ok) {
                throw new Error('Error en la solicitud');
            }
            return response.json();
        })
        .then(data => {
            // Mostrar la respuesta en el contenedor correspondiente
            const contenedorRespuesta = document.getElementById(contenedorRespuestaId);
            contenedorRespuesta.innerHTML = `
                <p>Respuesta del servidor: ${data.mensaje}</p>
            `;
        })
        .catch(error => {
            console.error('Error:', error);
            const contenedorRespuesta = document.getElementById(contenedorRespuestaId);
            contenedorRespuesta.innerHTML = '<p>Error al enviar el formulario.</p>';
        });
}

// Inicializar contenido dinámico y manejo de formularios
document.addEventListener('DOMContentLoaded', () => {
    // Inicializar contenedores dinámicos
    const elementosDinamicos = document.querySelectorAll('[data-id]');

    elementosDinamicos.forEach(elemento => {
        const id = elemento.getAttribute('data-id');
        const contenedorId = elemento.id;

        cargarContenido(id, contenedorId);
    });

    // Manejar el formulario al enviarlo
    const formulario = document.getElementById('miFormulario');
    formulario.addEventListener('submit', (event) => {
        event.preventDefault(); // Evitar recargar la página
        enviarFormulario('miFormulario', 'dyn', 'respuesta'); // Llamar a la función genérica
    });
});
