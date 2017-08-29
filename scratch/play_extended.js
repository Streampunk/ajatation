/* Copyright 2017 Streampunk Media Ltd.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

var H = require('highland');
var fs = require('fs');
var ajatation = require('../index.js');

var readdir = H.wrapCallback(fs.readdir);
var readFile = H.wrapCallback(fs.readFile);
var writeFile = H.wrapCallback(fs.writeFile);

var playback = new ajatation.Playback(0, ajatation.bmdModeHD1080i50, ajatation.bmdFormat10BitYUV);

var rootFolder = "E:/media/EBU_test_sets/filexchange.ebu.ch/EBU test sets - Creative Commons (BY-NC-ND)/HDTV test sequences/1080i25/";

var material = {
  queen: "C:/Users/zztop/Videos/queen2",
  sheep: "F:/media/streampunk/sheep",
  tree: "F:/media/streampunk/tree",
  view: "F:/media/streampunk/view"
};

var baseFolder = (process.argv[2] && material[process.argv[2]]) ?
  material[process.argv[2]] : material.graphics;

var count = 0;

var baseTime = process.hrtime();

H((push, next) => { push(null, baseFolder); next(); })
  .take((process.argv[3] && !isNaN(+process.argv[3]) && +process.argv[3] > 0) ?
    +process.argv[3] : 1)
  .flatMap(x => readdir(x).flatten().filter(y => y.endsWith('v210')).sort())
  .map(x => baseFolder + '/' + x)
  .map(x => readFile(x).map(y => ({ name: x, contents: y })))
  .parallel(10)
  .consume((err, x, push, next) => {
    if (err) { push(err); next(); }
    else if (x === H.nil) { push(null, H.nil); }
    else {
      var diffTime = process.hrtime(baseTime);
      var dtms = diffTime[0] * 1000 + diffTime[1] / 1000000|0;
      var wait = count * 40 - dtms;
      // console.log('dtms = ', dtms, 'so waiting', wait, 'at count', count);
      setTimeout(() => { push(null, x); next(); }, (wait > 0) ? wait : 0); }
  })
  .doto(x => { playback.frame(x.contents); })
  .doto(() => { if (count++ == 4) { playback.start(); } })
  .errors(H.log)
  .done(() => { playback.stop(); });

process.on('SIGINT', () => {
  playback.stop();
  process.exit();
});
