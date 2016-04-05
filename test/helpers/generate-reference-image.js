#!/usr/bin/env node
'use strict';
var Rsvg = require('../../index').Rsvg;
var fs = require('fs');

function renderSvg(svg, options) {
    return new Buffer(svg.render({
        format: options.format,
        width: options.width || svg.width,
        height: options.height || svg.height
    }).data);

}

function render(options) {
    options = options || {};
    options.format = options.format || 'png';

    var content = fs.readFileSync('./test/svg/smp.svg');

    var svg = new Rsvg(content);
    return renderSvg(svg, options);
}

fs.writeFile("./test/png/smp.png" , render(), function(err) {
    if(err) {
        return console.log(err);
    }
    console.log("The file was saved!");
}); 