with beatles as (
    select row_number() over (
            order by artist_alias.id desc
        ) as rank,
        artist_alias.name as name
    from artist_alias
        inner join artist on artist_alias.artist = artist.id
    where artist.name = "The Beatles"
),
cte(rank, name) as (
    select rank,
        name
    from beatles
    where rank = 1
    union all
    select beatles.rank,
        beatles.name || ', ' || cte.name
    from beatles
        inner join cte on cte.rank + 1 = beatles.rank
)
select name
from cte
order by rank desc
limit 1;