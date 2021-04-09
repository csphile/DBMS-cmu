select decade,
    count(*) as c
from (
        select (cast(r2.date_year / 10 as int) * 10) || 's' as decade
            from release r1
            inner join release_info r2 on r2.release = r1.id
        where r1.status = 1
            and r2.date_year >= 1900
    )
group by decade
order by c desc,
    decade desc;