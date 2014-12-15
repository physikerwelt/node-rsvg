
var assert = require('assert');
var child_process = require('child_process');

describe("Rsvg in some cases render zero-sized buffer", function() {

    it('Zero-sized buffer in two overlap cycles', function(cb) {
        var child = child_process.exec('node ./test/zerobuffer', function(err) {
            if (err) cb(err);
        });
        child.on('exit', function(code) {
            assert(code == 0, "Zero-size buffer detected");

            cb();
        });
    });
});

