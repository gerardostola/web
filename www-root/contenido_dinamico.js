function cargarContenido(id, contenedorId) {
    const url = `dyn?id=${id}`;
    
    fetch(url)
        .then(response => {
            if (!response.ok) {
                throw new Error('Error al cargar el contenido.');
            }
            return response.text();
        })
        .then(data => {
            const contenedor = document.getElementById(contenedorId);
            contenedor.innerHTML = data;
            //cargarOpciones();
        })
        .catch(error => {
            console.error('Error:', error);
            const contenedor = document.getElementById(contenedorId);
            contenedor.innerHTML = '<p>Error al cargar el contenido.</p>';
        });
}


function cargarOpciones() {
    const selects = document.querySelectorAll('select[data-sel]');

    selects.forEach(select => {
        const id = select.getAttribute('data-sel');
        const url = `dyn?sel=${id}`;
        fetch(url)
            .then(response => {
                if (!response.ok) {
                    throw new Error('Error al cargar las opciones.');
                }
                return response.json();
            })
            .then(data => {
                console.log(`Opciones recibidas para select con data-sel=${id}:`, data);

                // Limpiar el <select> antes de agregar nuevas opciones
                select.innerHTML = '';

                // Crear las opciones dinámicamente a partir de los datos recibidos
                data.opciones.forEach(opcion => {
                    const optionElement = document.createElement('option');
                    optionElement.value = opcion.value; // Valor del <option>
                    optionElement.textContent = opcion.text; // Texto visible

                    // Verificar si esta opción debe ser la seleccionada
                    if (opcion.value === data.seleccionado) {
                        optionElement.selected = true;
                    }

                    select.appendChild(optionElement);
                });
            })
            .catch(error => {
                console.error(`Error al cargar opciones para data-sel=${id}:`, error);
                // Mostrar un mensaje de error en el <select>
                select.innerHTML = '<option value="">Error al cargar las opciones</option>';
            });
    });
}






function enviarFormulario() {
    const form = document.getElementById('miFormulario1');
    const formData = new FormData(form);
    const data = {};

    formData.forEach((value, key) => {
        data[key] = value;
    });

    fetch('/contenido.html', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Error en la respuesta del servidor');
        }
        return response.json();
    })
    .then(data => {
        if (data.success) {
            mostrarMensaje('El formulario se envió correctamente.', 'success');
        } else {
            mostrarMensaje('Hubo un error al enviar el formulario.', 'error');
        }
    })
    .catch(error => {
        mostrarMensaje('Error de conexión con el servidor.', 'error');
    });
}

function mostrarMensaje(mensaje, tipo) {
    const mensajeDiv = document.createElement('div');
    mensajeDiv.className = `mensaje ${tipo}`;
    mensajeDiv.textContent = mensaje;
    document.body.appendChild(mensajeDiv);

    // Ocultar el mensaje después de 3 segundos
    setTimeout(() => {
        mensajeDiv.remove();
    }, 3000);
}

// Registrar el evento para cargar las opciones automáticamente al cargar la página
document.addEventListener('DOMContentLoaded', () => {
    cargarOpciones();

    const form = document.getElementById('miFormulario1');
    form.addEventListener('submit', function(event) {
        event.preventDefault(); // Evita el envío tradicional del formulario
        enviarFormulario();
    });


});
