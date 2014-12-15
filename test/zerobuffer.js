
var Rsvg = require('../index').Rsvg;
var fs = require('fs');

var N = 3220;

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

function st() {
    while (N-- > 0) {
        // process.stdout.write(".");
        // console.log('iteration', N);
        var rendered = render();

        if (rendered.length == 0) {
            console.error('failed');
            process.exit(1);
            //break;
        }
    }
}

st();

process.nextTick(st);