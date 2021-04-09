with artist_target(id) as (
    select id
    from artist_credit
    where artist_credit.name like "%Ariana Grande%"
)
select count(distinct artist_credit_name.artist)
from artist_credit_name,
    artist_target
where artist_credit_name.artist_credit = artist_target.id;