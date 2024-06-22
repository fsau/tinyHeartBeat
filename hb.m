s1 = [];
s2 = [];
s3 = [];
timePeriod = 2000;

for i = 0:(3*timePeriod/40)
    y=sin(i/(timePeriod/20));
    d = 430-(y*100);
    t=350+y*50;
    s1 = [s1 d];
    s2 = [s2 d];
    s3 = [s3 t];
end

for i=0:(3*timePeriod/400)
    y=sin(i/(timePeriod/20));
    d=430;
    t=350;
    s1 = [s1 d];
    s2 = [s2 s2(end)];
    s3 = [s3 t];
end

for i=0:((7*timePeriod)/400)
    y=sin(i/(timePeriod/20));
    d=430;
    % t=350+y*50;
    s1 = [s1 s1(end)];
    s2 = [s2 d];
    s3 = [s3 t];
end

s1=[s1,s1,s1];
s2=[s2,s2,s2];
s3=[s3,s3,s3];

hold off
plot(1/60*round(s1)/2^12,'marker','+')
hold on
plot(1/60*round(s2)/2^12,'marker','*')
plot(1/60*round(s3)/2^12,'marker','o')