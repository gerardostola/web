function fillOne(id, contenedorId) {
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
        })
        .catch(error => {
            console.error('Error:', error);
            const contenedor = document.getElementById(contenedorId);
            contenedor.innerHTML = '<p>Error al cargar el contenido.</p>';
        });
}

function fillAll() {

	// checkboxes		
	 const checkboxes = document.querySelectorAll('input[type="checkbox"]');

    checkboxes.forEach(checkbox => {
        //const id = checkbox.getAttribute('data-chk');        
  	    const id = checkbox.id; // Usamos el atributo id directamente
				const url = `dyn?id=${id}`;
        fetch(url)
            .then(response => {
                if (!response.ok) {
                    throw new Error('checkbox request error');
                }
                return response.json();
            })
            .then(data => {
                checkbox.checked = data.checked;
            })
            .catch(error => {
                console.error(`check ${id}:`, error);
            });
    });
	
		// selects
    const selects = document.querySelectorAll('select');

    selects.forEach(select => {
        //const id = select.getAttribute('data-sel');
				const id = select.id; 
        const url = `dyn?id=${id}`;
        fetch(url)
            .then(response => {
                if (!response.ok) {
                    throw new Error('Error on select options.');
                }
                return response.json();
            })
            .then(data => {
                select.innerHTML = '';

                data.opciones.forEach(opcion => {
                    const optionElement = document.createElement('option');
                    optionElement.value = opcion.value;
                    optionElement.textContent = opcion.text;

                    if (opcion.value === data.sel) {
                        optionElement.selected = true;
                    }

                    select.appendChild(optionElement);
                });
            })
            .catch(error => {
                console.error(`Error sel ${id}:`, error);
                select.innerHTML = '<option value="">Error loading options</option>';
            });
    }); 
}


function fillAll2() {


		const allIds = Array.from(document.querySelectorAll('input[type="checkbox"][id], select[id]'))
				.map(element => parseInt(element.id, 10));

		const requestData = {
				get: allIds
		};


    fetch('dyn', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(requestData)
    })
        .then(response => {
            if (!response.ok) {
                throw new Error('Error al obtener los datos del servidor');
            }
            return response.json();
        })
        .then(data => {
            data.checkboxes.forEach(item => {
                const checkbox = document.getElementById(item.id);
                if (checkbox) {
                    checkbox.checked = item.checked;
                }
            });

            data.selects.forEach(item => {
                const select = document.getElementById(item.id);
                if (select) {
                    select.innerHTML = ''; // Limpiar el <select>
                    item.opciones.forEach(opcion => {
                        const optionElement = document.createElement('option');
                        optionElement.value = opcion.value;
                        optionElement.textContent = opcion.text;

                        if (opcion.value === item.sel) {
                            optionElement.selected = true;
                        }

                        select.appendChild(optionElement);
                    });
                }
            });
        })
        .catch(error => {
            console.error('Error al cargar los datos:', error);
        });
}

function enviarFormulario(form) {
    const formData = new FormData(form);
    const formattedData = { set: [] };
		const uniqueFields = new Map();

    formData.forEach((value, key) => {
        uniqueFields.set(key, value);
    });

    uniqueFields.forEach((value, key) => {
        //const idNumber = parseInt(key.split('-')[1], 10);
				const idNumber = parseInt(key, 10);
        formattedData.set.push({ id: idNumber, value: value });
    });

		const action = form.getAttribute('action');
		
    fetch(action, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formattedData)
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
    setTimeout(() => {
        mensajeDiv.remove();
    }, 3000);
}


document.addEventListener('DOMContentLoaded', () => {
		fillAll2();
    
		const forms = document.querySelectorAll('form');
    forms.forEach(form => {
        form.addEventListener('submit', function(event) {
            event.preventDefault();
            enviarFormulario(form);
        });
    });
});
