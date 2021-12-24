"use strict";

const vsSource = `#version 300 es

in vec2 vertexPosition;
in vec2 textureCoords;

out vec2 texCoord;

uniform vec2 shift;
uniform float scale;

void main() {
    gl_Position = vec4(vertexPosition * scale + shift, 0.0, 1.0);
    texCoord = textureCoords;
}
`;

const fsSource = `#version 300 es
precision mediump float;

in vec2 texCoord;

out vec4 color;

uniform sampler2D textureData;

void main() {
    color = texture(textureData, texCoord);
}
`;

window.onload = function main() {
    const canvas = document.querySelector("#gl_canvas");
    const gl = canvas.getContext("webgl2");

    if (gl === null) {
        alert("Unable to initialize WebGL. Your browser or machine may not support it.");
        return;
    }
    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);

    const shaderProgram = initShaderProgram(gl, vsSource, fsSource);
    const programInfo = {
        program: shaderProgram,
        attribLocations: {
            vertexPosition: gl.getAttribLocation(shaderProgram, 'vertexPosition'),
            textureCoordBuffer: gl.getAttribLocation(shaderProgram, 'textureCoords'),
        },
        uniformLocations: {
            textureData: gl.getUniformLocation(shaderProgram, 'textureData'),
            shift: gl.getUniformLocation(shaderProgram, 'shift'),
            scale: gl.getUniformLocation(shaderProgram, 'scale'),
        },
        textures: {
            texture: loadTexture(gl, "/image.jpg")
        }
    };

    const buffers = initBuffer(gl)

    let tick = 0;
    setInterval(() => {
        drawScene(gl, programInfo, buffers, tick++)
    }, 1000 / 60);
}

function loadShader(gl, type, source) {
    const shader = gl.createShader(type);

    gl.shaderSource(shader, source);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        alert('An error occurred compiling the shaders: ' + gl.getShaderInfoLog(shader));
        gl.deleteShader(shader);
        return null;
    }

    return shader;
}

function initShaderProgram(gl, vsSource, fsSource) {
    const vertexShader = loadShader(gl, gl.VERTEX_SHADER, vsSource);
    const fragmentShader = loadShader(gl, gl.FRAGMENT_SHADER, fsSource);

    const shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        alert('Unable to initialize the shader program: ' + gl.getProgramInfoLog(shaderProgram));
        return null;
    }

    return shaderProgram;
}

function loadTexture(gl, url) {
    const texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texture);

    const level = 0;
    const internalFormat = gl.RGBA;
    const width = 1;
    const height = 1;
    const border = 0;
    const srcFormat = gl.RGBA;
    const srcType = gl.UNSIGNED_BYTE;

    const pixel = new Uint8Array([0, 0, 255, 255]);
    gl.texImage2D(gl.TEXTURE_2D, level, internalFormat, width, height, border, srcFormat, srcType, pixel);

    const image = new Image();
    image.onload = () => {
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.texImage2D(gl.TEXTURE_2D, level, internalFormat, srcFormat, srcType, image);
        gl.generateMipmap(gl.TEXTURE_2D);
    };
    image.src = url;

    return texture;
}

function initBuffer(gl) {

    const points = [
        [-1, -1],
        [-1, +1],
        [+1, +1],

        [-1, -1],
        [+1, +1],
        [+1, -1],
    ]

    const positions = points.flat()

    const textureCoords = [
        [0.0, 1.0],
        [0.0, 0.0],
        [1.0, 0.0],

        [0.0, 1.0],
        [1.0, 0.0],
        [1.0, 1.0],
    ].flat()


    const positionBuffer = makeF32ArrayBuffer(gl, positions);
    const textureCoordBuffer = makeF32ArrayBuffer(gl, textureCoords);

    return {
        positionBuffer,
        textureCoordBuffer,
        bufferLength: points.length,
    };
}

function makeF32ArrayBuffer(gl, array) {
    const buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);

    gl.bufferData(
        gl.ARRAY_BUFFER,
        new Float32Array(array),
        gl.STATIC_DRAW
    );

    return buffer
}

function drawScene(gl, programInfo, buffers, tick) {
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    drawBuffers(gl, programInfo, buffers, [0, evaluateYShift(0, tick)], 0.5)
    drawBuffers(gl, programInfo, buffers, [0, evaluateYShift(1, tick)], 0.5)
    drawBuffers(gl, programInfo, buffers, [0, evaluateYShift(2, tick)], 0.5)
}

function evaluateYShift(index, tick) {
    let frequency = 100
    let numberOfSquares = 3
    return 1.5 - ((tick + (frequency * index)) % (numberOfSquares * frequency)) / frequency
}

function drawBuffers(gl, programInfo, buffers, shift, scale) {
    gl.bindBuffer(gl.ARRAY_BUFFER, buffers.positionBuffer);
    gl.vertexAttribPointer(programInfo.attribLocations.vertexPosition, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(programInfo.attribLocations.vertexPosition);

    gl.bindBuffer(gl.ARRAY_BUFFER, buffers.textureCoordBuffer);
    gl.vertexAttribPointer(programInfo.attribLocations.textureCoordBuffer, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(programInfo.attribLocations.textureCoordBuffer);

    gl.useProgram(programInfo.program);
    gl.uniform1i(programInfo.uniformLocations.textureData, 0)
    gl.uniform2fv(programInfo.uniformLocations.shift, shift)
    gl.uniform1f(programInfo.uniformLocations.scale, scale)
    gl.drawArrays(gl.TRIANGLES, 0, buffers.bufferLength);
}